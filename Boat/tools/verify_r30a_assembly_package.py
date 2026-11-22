#!/usr/bin/env python3
"""R30A authoritative unpowered assembly handoff checks.

This script checks DOCUMENT and SOFTWARE consistency, never physical
electrical acceptance or permission to connect MDD20A/EKM014 load power.
"""
from __future__ import annotations
import csv
from collections import Counter
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]

def rows(path):
    with (ROOT / path).open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        assert reader.fieldnames is not None, path
        data = list(reader)
    assert data and all(all(v is not None for v in r.values()) for r in data), path
    return data

def check_netlist(r23, r27, r29, effective):
    assert len(r23) == 64 and len(r27) == 10 and len(r29) == 8
    retired = {r["Net"] for r in r23 if
               r["Harness_ID"] == "H03_ADS" or
               r["Net"] in {"MDD_MOTOR_SUPPLY", "EKM_VH"}}
    assert len(retired) == 9
    retained = {r["Net"]:r for r in r23 if r["Net"] not in retired}
    assert len(retained) == 55
    assert len(effective) == 73, len(effective)
    assert len({r["Net"] for r in effective}) == len(effective)
    e = {r["Net"]:r for r in effective}
    assert retired.isdisjoint(e)
    assert set(e) == set(retained) | {r["Net"] for r in r27} | {r["Net"] for r in r29}
    assert Counter(r["Install_Status"] for r in effective) == {
        "UNWIRED_PENDING":64, "NO_WIRE":7, "REFERENCE_ONLY":2}
    assert all(r["Electrical_Gate"] and r["Authority"] for r in effective)
    for net, old in retained.items():
        now=e[net]
        assert now["Authority"]=="R23_RETAINED"
        assert (now["Source"],now["Source_Pin"],now["Destination"],now["Destination_Pin"]) == (
            old["Source"],old["Source_Pin"],old["Destination"],old["Destination_Pin"])
    for r in r27:
        now=e[r["Net"]]
        assert now["Authority"]=="R27_REPLACES_ADS"
        assert now["Install_Status"] == ("NO_WIRE" if r["Design_Status"]=="NO_WIRE" else "UNWIRED_PENDING")
        assert now["Source"]==r["Origin"] and now["Destination"]==r["Destination"]
    for r in r29:
        now=e[r["Net"]]
        alias = r["Net"] in {"INA231_BUS_SENSE","EKM_LOGIC_VL"}
        assert now["Authority"]==("R29_ALIAS_ONLY" if alias else "R29_REPLACES_VM_VH")
        assert now["Install_Status"]==("REFERENCE_ONLY" if alias else "UNWIRED_PENDING")
        assert now["Source"]==r["Source"] and now["Destination"]==r["Destination"]
    # One common 3S+ and return, not independently switched motor/coil feeds.
    assert e["MDD_MOTOR_VM"]["Source"] == e["EKM_FET_VH"]["Source"] == "COMMON_3S_DISTRIBUTION"
    assert e["MDD_MOTOR_GND"]["Source"] == e["EKM_FET_GND"]["Source"] == "COMMON_3S_DISTRIBUTION"
    assert e["INA231_BUS"]["Install_Status"] == "UNWIRED_PENDING"
    assert e["INA231_SHUNT_PLUS"]["Install_Status"] == "NO_WIRE"
    assert e["INA231_SHUNT_MINUS"]["Install_Status"] == "NO_WIRE"
    assert e["INA231_BUS_SENSE"]["Install_Status"] == "REFERENCE_ONLY"
    assert e["EKM_LOGIC_VL"]["Install_Status"] == "REFERENCE_ONLY"
    return True

def check_readiness(gates):
    assert len(gates) == 24
    assert len({r["Gate_ID"] for r in gates}) == len(gates)
    assert all(r["State"]=="OPEN" and r["Required_Evidence"] and
               r["Evidence_Class"] and r["Acceptance_Requirement"]
               for r in gates)
    assert {"ACTUATOR","LOGIC","WATER","UNPOWERED"} == {
        r["Earliest_Phase"] for r in gates}
    assert {"G11","G13","G14","G15","G17","G24"}.issubset({
        r["Gate_ID"] for r in gates if r["State"]=="OPEN"})
    return True

def main():
    r23=rows("Boat/docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv")
    r27=rows("Boat/docs/BOAT_R27_INA231_HARNESS_OVERRIDE.csv")
    r29=rows("Boat/docs/BOAT_R29_SHARED_3S_BUS_CONNECTIONS.csv")
    effective=rows("Boat/docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv")
    assert check_netlist(r23,r27,r29,effective)
    gates=rows("Boat/docs/BOAT_R30A_READINESS_GATES.csv")
    assert check_readiness(gates)
    f=lambda path:(ROOT/path).read_text(encoding="utf-8")
    doc=f("Boat/docs/BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md")
    assert "NOT yet approved for live actuator-power wiring" in doc
    assert "Flytec_Boat.ino" in doc and "R100" in doc and "VER014" in doc
    src=f("Boat/source/firmware/Flytec_Boat.ino")
    assert "application.tick();" in src and "No physical stationary/STOP sensor" in src
    assert "application.attach(" not in src
    radio=f("Boat/source/boat_radio_hw_config.h")
    assert "#define BOAT_NRF_PIN_AUDIT_SIGNED 0" in radio
    assert "#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0" in radio
    ina=f("Boat/source/ina231_voltage_contract_r27.h")
    assert "INA231_PHYSICAL_COMMISSIONED=false" in ina
    bus=f("Boat/source/boat_r29_shared_bus_contract.h")
    assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in bus
    assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in bus
    driver=f("Boat/source/boat_physical_driver_adapter.h")
    assert "!independentHardwareCommissioned" in driver
    bench=f("Boat/hardware_test/INA231_Battery_Bench/INA231_Battery_Bench.ino")
    assert "static_assert(!boat_r27::INA231_PHYSICAL_COMMISSIONED" in bench
    assert "Wire.begin(" in bench and "if (!eligible)" in bench
    for forbidden in ["mdd20a", "EKM014", "motorWrite(", "coilWrite(", "analogWrite("]:
        assert forbidden.lower() not in bench.split("#include",1)[1].lower() or forbidden=="EKM014", forbidden
    for name,column,count in [
        ("Boat/docs/BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv","Test_Status",32),
        ("Boat/docs/BOAT_R25_POWER_LEVEL_EVIDENCE.csv","Status",28),
        ("Boat/docs/BOAT_R27_INA231_ACCEPTANCE.csv","Status",21)]:
        x=rows(name)
        assert len(x)==count and all(r[column]=="NOT_RUN" for r in x)
    print("R30A document consistency PASS: 73 effective nets (64 unwired, 7 NC, 2 references), 24 OPEN gates, 81 NOT_RUN physical checks, locked Boat firmware")

if __name__=="__main__":
    main()
