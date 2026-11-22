#!/usr/bin/env python3
"""R30H-2 BOAT-LOCAL serial-event consistency checker (NEVER RF signoff).

Accepts lines emitted only by optional, nonactuating Flytec_Boat diagnostic
target: R30H2,BOAT_LOCAL,ms,last_rx_ms,event,link,mask,command_id,overflows,INTENT_ONLY.
Even with a complete log, only local intervals can be calculated; there is no
Remote clock, physical output-current evidence, or real RF link endorsement.
"""
import argparse
import csv
import sys

PREFIX = ("R30H2", "BOAT_LOCAL")
EVENTS = {
    "SESSION_ESTABLISHED", "AUTH_PRIMARY_SAMPLE", "AUTH_BACKUP_SAMPLE",
    "BOTH_LINKS_STALE", "AUTH_LINK_RESTORED", "STOP_AUTH_RECEIVED",
    "STOP_LOGICAL_INTENT", "RTH_LOCAL_ENTRY",
}
MAX_U32 = 0xFFFFFFFF

def delta_ms(then, now):
    d = (now - then) & MAX_U32
    if d > 0x7FFFFFFF:
        raise ValueError("reversed/ambiguous millis() timestamps (>half-wrap)")
    return d

def parse(lines):
    out = []
    for lineno, line in enumerate(lines, 1):
        if not line.startswith("R30H2,"):
            continue
        fields = next(csv.reader([line]))
        if len(fields) != 10 or tuple(fields[:2]) != PREFIX or fields[-1] != "INTENT_ONLY":
            raise ValueError(f"line {lineno}: invalid R30H2 schema / attempted physical label")
        _, _, now, last, event, link, mask, command, drops, _ = fields
        if event not in EVENTS:
            raise ValueError(f"line {lineno}: unknown event {event}")
        try:
            ns = [int(v) for v in (now, last, link, mask, command, drops)]
        except ValueError as exc:
            raise ValueError(f"line {lineno}: non-integer record field") from exc
        if any(v < 0 for v in ns) or ns[0] > MAX_U32 or ns[1] > MAX_U32 or ns[2] > 2 or ns[3] > 3 or ns[4] > 65535 or ns[5] > MAX_U32:
            raise ValueError(f"line {lineno}: out-of-range field")
        out.append({"now": ns[0], "last": ns[1], "event": event,
                    "link": ns[2], "mask": ns[3], "command": ns[4], "drops": ns[5]})
    return out

def analyze(records):
    if not records:
        return "NOT_RUN", [], {}
    errors = []
    pending_stop = {}
    last_loss = None
    loss_active = False
    stop_intervals, rth_age, loss_to_rth = [], [], []
    previous = None
    for r in records:
        e = r["event"]
        if r["drops"]:
            errors.append("trace overflow: incomplete record history, cannot sign timing")
        if previous is not None:
            try:
                delta_ms(previous["now"], r["now"])
            except ValueError:
                errors.append("events out of order or BOAT restart between records")
        previous = r
        if e == "SESSION_ESTABLISHED":
            pending_stop.clear()
            loss_active = False
            last_loss = None
        elif e == "STOP_AUTH_RECEIVED":
            if not r["command"]:
                errors.append("STOP command has no ID")
            pending_stop[r["command"]] = r["now"]
        elif e == "STOP_LOGICAL_INTENT":
            origin = pending_stop.pop(r["command"], None)
            if origin is None:
                errors.append(f"logical STOP has no matching authenticated command {r['command']}")
            else:
                try:
                    stop_intervals.append(delta_ms(origin, r["now"]))
                except ValueError:
                    errors.append("logical STOP precedes authenticated STOP")
        elif e == "BOTH_LINKS_STALE":
            loss_active = True
            last_loss = r["now"]
            if r["mask"] != 0:
                errors.append("both stale recorded with fresh RF path")
        elif e == "AUTH_LINK_RESTORED":
            loss_active = False
            last_loss = None
            if r["mask"] == 0:
                errors.append("recovery without any fresh path")
        elif e == "RTH_LOCAL_ENTRY":
            if not loss_active or last_loss is None:
                errors.append("RTH without recorded both-links-stale phase")
            else:
                try:
                    loss_to_rth.append(delta_ms(last_loss, r["now"]))
                except ValueError:
                    errors.append("RTH before recorded total loss")
            try:
                age = delta_ms(r["last"], r["now"])
                rth_age.append(age)
                if age < 10000:
                    errors.append(f"RTH EARLY: {age}ms < 10000ms after last accepted packet")
            except ValueError:
                errors.append("RTH last authenticated RX timestamp ambiguous")
            if r["mask"] != 0:
                errors.append("RTH with fresh radio path")
    if pending_stop:
        errors.append("missing logical STOP intent for authenticated STOP")
    if errors:
        return "INVALID_OR_INCOMPLETE", errors, {}
    return "BOAT_LOCAL_OBSERVATIONS_ONLY", [], {
        "stop_auth_to_intent_ms": stop_intervals,
        "last_auth_to_rth_ms": rth_age,
        "both_stale_to_rth_ms": loss_to_rth,
    }

def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("logfile", help="USB-local serial text from nonactuating Boat trace")
    args=parser.parse_args(argv)
    try:
        with open(args.logfile, encoding="utf-8") as stream:
            rows=parse(stream)
        status, errors, measurements=analyze(rows)
    except (OSError,ValueError) as exc:
        print("INVALID_LOG:",exc)
        return 2
    print(f"R30H-2 {status}: records={len(rows)}")
    for metric, values in measurements.items():
        if values:
            print(f"  {metric}: n={len(values)} min={min(values)} max={max(values)} (BOAT_LOCAL millis, NOT RF E2E)")
    for error in errors:
        print("  ERROR:", error)
    print("Physical STOP, current-zero and RF delivery: NOT_MEASURED; signed real-RF trials: NOT_RUN")
    return 3 if status=="NOT_RUN" else 2 if errors else 0

if __name__=="__main__":
    sys.exit(main())
