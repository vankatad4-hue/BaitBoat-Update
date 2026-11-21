#!/usr/bin/env python3
"""Validate KiCad R9 COIL schematic topology, not hardware safety or ratings."""
import argparse
import pathlib
import re
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCHEMATIC = ROOT / "hardware/Coil_Drivers_R9/Boat_Four_Coil_Drivers_R9.kicad_sch"
CHANNELS = ("LH", "RH", "LK", "RK")


def source_checks():
    data = SCHEMATIC.read_text(encoding="utf-8")
    expected_refs = {"J1", "J2", *("J" + str(i) for i in range(3, 7)),
                     *("F" + str(i) for i in range(1, 5)),
                     *("Q" + str(i) for i in range(1, 5)),
                     *("D" + str(i) for i in range(1, 5)),
                     *("R" + str(i) for i in range(1, 9))}
    refs = re.findall(r'\(symbol \(lib_id "Flytec_R9:[^"]+"\).*?\(property "Reference" "([^"]+)"', data)
    assert len(refs) == len(expected_refs) == 26, refs
    assert set(refs) == expected_refs, (set(refs), expected_refs)
    assert data.count('(property "Footprint" ""') == 26, "Footprints must NOT be approved"
    assert 'R9-DRAFT-NO-FAB' in data
    assert "HIGH-SIDE STOP/WATCHDOG LATCH AND ONE-HOT" in data
    for ch in CHANNELS:
        for net in (f"PULSE_{ch}_LIMITED_TBD", f"SOL_{ch}_PLUS_FUSED",
                    f"SOL_{ch}_SW", f"GATE_{ch}"):
            assert data.count('(label "' + net + '"') >= 2, net
    assert all(x not in data for x in ("RP2040", "SONAR", "RUDDER", "E22"))
    print("26 native KiCad instances, 4 channels, unassigned footprints: source OK")


def exported_netlist(path):
    doc = ET.parse(path)
    circuits = {}
    for net in doc.findall("./nets/net"):
        # KiCad 9 XML uses root-sheet-qualified local labels, e.g. /ACT_RETURN.
        netname = net.get("name", "").removeprefix("/")
        assert netname not in circuits, netname
        circuits[netname] = {(p.get("ref"), p.get("pin")) for p in net.findall("node")}
    return circuits


def expect(netlist, name, exact):
    actual = netlist.get(name)
    want = set(exact)
    assert actual is not None, "Missing net " + name
    assert actual == want, f"{name} actual={sorted(actual)} expected={sorted(want)}"


def netlist_checks(path):
    nets = exported_netlist(path)
    expect(nets, "ACT_SOL_POS_AFTER_LATCH_TBD",
           {("J1", "1"), *(("F" + str(i), "1") for i in range(1, 5))})
    expect(nets, "ACT_RETURN",
           {("J1", "2"), *(("Q" + str(i), "3") for i in range(1, 5)),
            *(("R" + str(i), "2") for i in range(2, 9, 2))})
    for i, ch in enumerate(CHANNELS, start=1):
        j, f, q, d = "J" + str(i + 2), "F" + str(i), "Q" + str(i), "D" + str(i)
        rg, rp = "R" + str(i * 2 - 1), "R" + str(i * 2)
        expect(nets, "SOL_" + ch + "_PLUS_FUSED",
               {(f, "2"), (j, "1"), (d, "1")})
        expect(nets, "SOL_" + ch + "_SW",
               {(j, "2"), (d, "2"), (q, "2")})
        expect(nets, "GATE_" + ch,
               {(rg, "2"), (rp, "1"), (q, "1")})
        expect(nets, "PULSE_" + ch + "_LIMITED_TBD",
               {("J2", str(i)), (rg, "1")})
    assert len(nets) == 18, f"Unexpected net count or cross-channel short: {sorted(nets)}"
    print("KiCad-exported electrical netlist: 18 nets, complete 4-channel topology OK")
    print("NOT VERIFIED: independent power cutoff, pulse limiter, feedback, ratings, power-on state")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--xml", type=pathlib.Path)
    args = parser.parse_args()
    source_checks()
    if args.xml:
        netlist_checks(args.xml)
