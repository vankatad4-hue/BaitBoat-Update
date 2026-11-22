#!/usr/bin/env python3
"""R30H-5 source audit: GPS UTC may update RTC, never monotonic STOP/RTH/RF leases.

This asserts source-level invariants only. It does not inspect a physical GNSS,
measure UART delay or RF latency, or claim PPS-quality clock synchronization.
"""
from pathlib import Path
import re
ROOT=Path(__file__).resolve().parents[2]
FILES=(
    "Boat/source/gnss_utc_sync.h","Boat/source/radio_utc_time_v1.h",
    "Remote/source/radio_utc_time_v1.h",
    "Boat/source/tracker_gnss.h","Boat/source/m100_ubx_nav_pvt.h",
    "Boat/source/tracker_sensor_port.h",
    "Boat/source/boat_dual_radio_port.cpp","Boat/source/firmware/Flytec_Boat.ino",
    "Remote/source/boat_radio.cpp","Remote/source/boat_radio.h",
    "Boat/source/boat_radio_hw_config.h","Remote/source/radio_hw_config.h",
    "Boat/source/boat_radio_safety.h",
)
def snapshot():
    return {name:(ROOT/name).read_text(encoding="utf8") for name in FILES}
def verify(source):
    errors=[]
    if set(source)!=set(FILES):
        return ["missing/unexpected UTC isolation sources"]
    def require(name,needle):
        if needle not in source[name]:errors.append(f"{name}: missing {needle!r}")
    if source["Boat/source/radio_utc_time_v1.h"]!=source["Remote/source/radio_utc_time_v1.h"]:
        errors.append("UTC wire/clock contract must match on both endpoints")
    wire=source["Boat/source/radio_utc_time_v1.h"]
    for t in ("SUBTYPE=9","VERSION=1","bool encode(uint64_t unixMs","bool decode(const uint8_t p[8]",
              "bool observe(uint64_t utcMs","DISCIPLINE_MAX_AGE_MS=20000U",
              "residual < -2000LL || residual > 2000LL","count_>=2U"):
        require("Boat/source/radio_utc_time_v1.h",t)
    boat=source["Boat/source/boat_dual_radio_port.cpp"]
    remote=source["Remote/source/boat_radio.cpp"]
    for t in ("flytec_utc::extrapolate(utcSource_,utcNow,ms)",
              "flytec_time::encode(ms,beacon.payload)","sendFrame(LINK_NRF_PRIMARY,beacon)",
              "!opportunity.activeMission","!opportunity.failsafeOrLinkLoss",
              "!opportunity.recentCommand","!nrf_.txPending()",
              "!diagnosticOccupiesSlot"):
        require("Boat/source/boat_dual_radio_port.cpp",t)
    if "sendFrame(LINK_LORA_BACKUP,beacon)" in boat:
        errors.append("UTC telemetry must never use provisional E22 airtime")
    for t in ("g_gpsUtcClock.observe(utcMs,now)","link!=LINK_NRF_PRIMARY",
              "frame.payload[0]==flytec_time::SUBTYPE",
              "g_gpsUtcClock.reset()","settimeofday(&tv,nullptr)",
              "g_linkManager.reportValidRx(link,true,now)"):
        require("Remote/source/boat_radio.cpp",t)
    guard_start=remote.find("if(!g_session.established()||frame.sessionEpoch!=g_sessionEpoch||!radio_frameVerify(frame,g_authKey))return;")
    payload_dispatch=remote.find("if(!dispatchValidatedPayload(link,frame,now))return;")
    skip_time=remote.find("frame.payload[0]==flytec_time::SUBTYPE))return;")
    report=remote.find("g_linkManager.reportValidRx(link,true,now);")
    if min(guard_start,payload_dispatch,skip_time,report)<0 or not guard_start<payload_dispatch<skip_time<report:
        errors.append("UTC receive must be HMAC/session-validated and NEVER refresh command-link freshness")
    for name in ("Boat/source/boat_dual_radio_port.cpp","Boat/source/boat_radio_safety.h"):
        if "settimeofday(" in source[name] or "gettimeofday(" in source[name]:
            errors.append(f"{name}: safety radio policy must never read/set wallclock")
    require("Boat/source/firmware/Flytec_Boat.ino","gnssUtcClock.observe(currentUtc,now)")
    require("Boat/source/firmware/Flytec_Boat.ino","radio.offerGnssUtc(utc)")
    require("Boat/source/firmware/Flytec_Boat.ino","settimeofday(&tv,nullptr)")
    require("Boat/source/tracker_sensor_port.h","utcSnapshot(uint32_t now)const")
    require("Boat/source/m100_ubx_nav_pvt.h","flytec_utc::utcFromUbx(")
    require("Boat/source/tracker_gnss.h","flytec_utc::utcFromNmeaDate(")
    if "application.attach(" in source["Boat/source/firmware/Flytec_Boat.ino"]:
        errors.append("RTC patch must not attach any actuator HAL")
    for name,flag in (
      ("Boat/source/boat_radio_hw_config.h","BOAT_NRF_PIN_AUDIT_SIGNED"),
      ("Boat/source/boat_radio_hw_config.h","BOAT_E22_RF_AIRTIME_AUDIT_SIGNED"),
      ("Remote/source/radio_hw_config.h","REMOTE_NRF_PIN_AUDIT_SIGNED"),
      ("Remote/source/radio_hw_config.h","REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED")
    ):
        if not re.search(r"^#define\s+"+flag+r"\s+0\s*$",source[name],re.M):
            errors.append(f"{name}: physical radio guard unexpectedly enabled")
    for token in ("(uint32_t)(now-lastNrfRxMs_)<=250",
                  "(uint32_t)(now-lastBackupRxMs_)<=6000",
                  "lastAnyAuthenticatedRxMs_,now,10000U"):
        if not re.search(re.escape(token)+r"(?![0-9])",boat):
            errors.append("radio freshness/RTH source timer changed unexpectedly")
    return errors
def main():
    failures=verify(snapshot())
    if failures:
        for x in failures:print("R30H-5 FAIL:",x)
        return 1
    print("R30H-5 UTC source isolation PASS (portable source-only, physical GPS and RF NOT_RUN)")
    return 0
if __name__=="__main__":
    raise SystemExit(main())
