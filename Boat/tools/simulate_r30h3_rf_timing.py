#!/usr/bin/env python3
"""R30H-3: deterministic *policy exposure* simulation. NOT RF measurements.

This script reads the current Boat and Remote source policy, enumerates every
remote E22 3s heartbeat phase and accepted-RX freshness age, then outputs
software-only bounds. No RF propagation, scheduler jitter, UART airtime,
acknowledgement, motor current or physical STOP timing is inferred.
"""
import argparse
import json
import math
import pathlib
import re
import sys
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[2]
REMOTE = ROOT / "Remote/source/boat_radio.cpp"
BOAT = ROOT / "Boat/source/boat_dual_radio_port.cpp"
RTH = ROOT / "Boat/source/boat_radio_safety.h"
E22 = ROOT / "Boat/source/e22_uart_link.h"

@dataclass(frozen=True)
class Policy:
    nrf_fresh_ms: int
    e22_fresh_ms: int
    e22_pace_ms: int
    rth_after_ms: int
    estimated_airtime_ms: int
    provisional_duty_budget_ms_per_hour: int


def number(pattern, data, label):
    m = re.search(pattern, data)
    if not m:
        raise ValueError(f"Cannot derive {label} from active source")
    return int(m.group(1))


def load_policy(root=ROOT):
    boat = (root / "Boat/source/boat_dual_radio_port.cpp").read_text()
    remote = (root / "Remote/source/boat_radio.cpp").read_text()
    rth = (root / "Boat/source/boat_radio_safety.h").read_text()
    e22 = (root / "Boat/source/e22_uart_link.h").read_text()
    primary = number(r"primaryFresh\(uint32_t now\)const\{[^\n]*<=\s*(\d+)U?;", boat, "Boat nRF freshness")
    backup = number(r"backupFresh\(uint32_t now\)const\{[^\n]*<=\s*(\d+)U?;", boat, "Boat E22 freshness")
    pace = number(r"g_backupTxSent&&now-g_lastBackupTxMs<(\d+)U", remote, "Remote STOP E22 pacing")
    if len(re.findall(r"g_pendingStop=command;\s*g_stopPending=true;", remote)) < 2:
        raise ValueError("Remote STOP pending semantics changed: review in source")
    if not re.search(r"if\(g_stopPending\)", remote):
        raise ValueError("Remote STOP is no longer prioritized before backup heartbeat")
    if not re.search(r"now-g_lastBackupTxMs>=(\d+)U", remote) or \
            number(r"now-g_lastBackupTxMs>=(\d+)U", remote, "Remote E22 heartbeat cadence") != pace:
        raise ValueError("Remote E22 STOP and heartbeat pacing have diverged")
    if not re.search(r"lastBackupHeartbeatTxMs_\)>=(\d+)U", boat):
        raise ValueError("Boat E22 heartbeat cadence not found")
    if number(r"lastBackupHeartbeatTxMs_\)>=(\d+)U", boat, "Boat E22 pacing") != pace:
        raise ValueError("Boat/Remote E22 heartbeat cadence differs")
    threshold = number(r"uint32_t timeoutMs=(\d+)U", rth, "RTH threshold")
    if number(r"lastAnyAuthenticatedRxMs_,now,(\d+)U", boat, "Boat RTH call") != threshold:
        raise ValueError("RTH helper threshold diverges from runtime")
    profile = (root / "Boat/source/lora_link_policy.h").read_text()
    remote_profile = (root / "Remote/source/lora_link_policy.h").read_text()
    if profile != remote_profile or "EST_AIRTIME_MS=flytec_rtos::LoRaAirtimeProfile::BaselinePacket;" not in e22 or "MAX_AIRTIME_PER_HOUR_MS=flytec_rtos::LoRaAirtimeProfile::BaselineHour;" not in e22:
        raise ValueError("E22 baseline profile/alias mismatch")
    est = number(r"BaselinePacket=(\d+)", profile, "E22 estimated airtime")
    budget = number(r"BaselineHour=(\d+)", profile, "E22 budget")
    return Policy(primary, backup, pace, threshold, est, budget)


def first_stale_ms(last_accepted, timeout_ms):
    """Return first millisecond when a last-accepted frame is no longer fresh."""
    return last_accepted + timeout_ms + 1


def software_loss_stop_ms(last_nrf, last_e22, at_or_after=0, primary_present=True, backup_present=True):
    deadlines = []
    if primary_present:
        deadlines.append(first_stale_ms(last_nrf, POLICY.nrf_fresh_ms))
    if backup_present:
        deadlines.append(first_stale_ms(last_e22, POLICY.e22_fresh_ms))
    return max([at_or_after] + deadlines)


def remote_stop_wait_ms(last_e22_tx, requested_at, pace):
    """Minimum wait imposed by app pacing; excludes UART/air/retries/ACK."""
    if requested_at >= last_e22_tx + pace:
        return 0
    return last_e22_tx + pace - requested_at


def exposure(policy):
    """1ms phase sweep, synthetic no-jitter timers; not physical time."""
    stops = [remote_stop_wait_ms(0, phase, policy.e22_pace_ms)
             for phase in range(policy.e22_pace_ms + 1)]
    # RF cut at t=0; latest valid E22 was 0..(pace-1) ms earlier.
    stale_e22 = [first_stale_ms(-age, policy.e22_fresh_ms)
                 for age in range(policy.e22_pace_ms)]
    both_stale = [max(first_stale_ms(0, policy.nrf_fresh_ms), x)
                  for x in stale_e22]
    return {
        "scenario": "DETERMINISTIC_SOFTWARE_ONLY_NO_RF",
        "remote_e22_stop_queue_wait_ms": {"min": min(stops), "max": max(stops),
                                           "examples": {"just_after_tx": stops[0], "just_before_next_slot": stops[-2], "at_next_slot": stops[-1]}},
        "both_links_stale_after_cut_ms": {"min": min(both_stale), "max": max(both_stale)},
        "nrf_only_first_stale_after_cut_ms": first_stale_ms(0, policy.nrf_fresh_ms),
        "e22_only_first_stale_after_cut_ms": first_stale_ms(0, policy.e22_fresh_ms),
        "earliest_rth_after_last_auth_ms": policy.rth_after_ms,
        "minimum_rth_interlocks": "already active eligible mission + CRC valid matching Home + local fresh navigation + physical safety; otherwise STOP",
        "real_rf_or_physical_output_latency_ms": None,
    }


def hypothetical_budget(policy, candidate_ms):
    if candidate_ms is None:
        return {"status": "NOT_MEASURED", "measured_airtime_ms": None}
    if not math.isfinite(candidate_ms) or candidate_ms <= 0:
        raise ValueError("Airtime candidate must be finite and positive")
    per_hour = 3600000.0 / policy.e22_pace_ms
    expected_ms = candidate_ms * per_hour
    return {
        "status": "HYPOTHETICAL_INPUT_NOT_PHY_MEASUREMENT",
        "candidate_airtime_ms": candidate_ms,
        "periodic_frames_per_hour_per_endpoint_if_no_drops": per_hour,
        "periodic_airtime_ms_per_hour": expected_ms,
        "provisional_software_budget_ms_per_hour": policy.provisional_duty_budget_ms_per_hour,
        "provisional_remaining_ms_for_commands": policy.provisional_duty_budget_ms_per_hour - expected_ms,
        "warning": "No legal channel/duty signoff; actual PHY, antenna EIRP, TX backoffs and retries unmeasured"
    }


POLICY = load_policy()


def main():
    arg = argparse.ArgumentParser(description=__doc__)
    arg.add_argument("--hypothetical-airtime-ms", type=float, default=None,
                     help="What-if only, NEVER RF measurement; no acceptance threshold")
    args = arg.parse_args()
    try:
        budget = hypothetical_budget(POLICY, args.hypothetical_airtime_ms)
    except ValueError as exc:
        arg.error(str(exc))
    print(json.dumps({"policy_from_live_source": POLICY.__dict__,
                      "software_exposure": exposure(POLICY),
                      "per_endpoint_provisional_airtime_budget": budget,
                      "physical_RF_and_STOP_acceptance": "NOT_RUN",
                      "motor_solenoid_permission": "BLOCKED"},
                     indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
