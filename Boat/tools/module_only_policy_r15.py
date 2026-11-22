#!/usr/bin/env python3
"""R15 repository-wide hardware-module-only policy guard.
A source/documentation check is NOT physical acceptance or a safety certificate.
"""
import csv
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
POLICY=ROOT/"docs/HARDWARE_MODULE_ONLY_POLICY_R15.md"
BOAT=ROOT/"Boat/docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md"
REMOTE=ROOT/"Remote/docs/REMOTE_MODULE_ONLY_IMPLEMENTATION_R15.md"
BOM=ROOT/"docs/MODULE_ONLY_BOM_R15.csv"
BW=ROOT/"Boat/docs/BOAT_MODULE_ONLY_WIRING_R15.csv"
RW=ROOT/"Remote/docs/REMOTE_MODULE_ONLY_WIRING_R15.csv"
HISTORICAL_WORKFLOWS=[
    ".github/workflows/boat-kicad-r7.yml",
    ".github/workflows/boat-kicad-coils-r9.yml",
    ".github/workflows/boat-kicad-safety-r10.yml",
    ".github/workflows/boat-kicad-pulse-r11.yml",
    ".github/workflows/boat-kicad-release-r12.yml",
    ".github/workflows/boat-kicad-power-r13.yml",
]
def load_csv(p):
    with p.open(newline="",encoding="utf-8") as f:
        return list(csv.DictReader(f))

def guard():
    policy=POLICY.read_text(encoding="utf-8")
    assert "NO custom electronic PCB design" in policy
    assert "Remote VER028" in policy and "Boat VER014" in policy
    for path in (BOAT,REMOTE):
        txt=path.read_text(encoding="utf-8")
        assert "No" in txt or "NO" in txt
    for wf in HISTORICAL_WORKFLOWS:
        c=(ROOT/wf).read_text(encoding="utf-8")
        assert "ARCHIVED R15" in c
        assert "workflow_dispatch:" in c
        assert "  push:" not in c and "  pull_request:" not in c
    bom=load_csv(BOM)
    assert len(bom)>=20
    assert {x["Side"] for x in bom}=={"Boat","Remote","Both"}
    assert not any("APPROVED_FOR_LIVE_ACTUATION" in r["State"] for r in bom)
    required={
        ("Boat","Controller/GNSS/LoRa"),("Boat","Brushed dual motor driver"),
        ("Boat","Four solenoid low-side power"),("Boat","Safety logic supply 3.3V"),
        ("Boat","Safety DC disconnect"),("Boat","Hardware timer and one-hot"),
        ("Boat","Independent POR and watchdog"),("Boat","Current zero sensors"),
        ("Remote","Controller touchscreen SD"),("Remote","16 key GPIO breakout"),
        ("Remote","4 joystick axes ADC"),("Remote","Battery and NTC ADC"),
        ("Remote","nRF PA LNA power"),("Remote","E22 5V power"),
    }
    assert required<={(r["Side"],r["Function"]) for r in bom}
    for row in bom:
        assert row["Manufacturer_Reference"] or row["State"].startswith(("REQUIRED_","CANDIDATE","RETAINED_CANDIDATE")),row
    boat=load_csv(BW)
    remote=load_csv(RW)
    assert len(boat)>=16 and len(remote)>=12
    assert not any(r["Commissioning_Gate"].strip()=="" or r["Electrical_State"].startswith("COMMISSIONED") for r in boat)
    assert not any(r["Status"].startswith("COMMISSIONED") for r in remote)
    assert any("EKM014" in r["To_Module"] and r["To_Port"]=="VL" and "5V" in r["Signal"] for r in boat)
    assert not any("EKM014" in r["To_Module"] and r["To_Port"]=="VL" and "3S" in r["Signal"] for r in boat)
    assert any(r["To_Module"]=="SC16IS750 premade breakout" and "0x48" in r["I2C_Address_or_voltage"] for r in remote)
    assert any("ADS1115 #1" in r["To_Module"] and "0x49" in r["I2C_Address_or_voltage"] for r in remote)
    assert any("ADS1115 #2" in r["To_Module"] and "0x4A" in r["I2C_Address_or_voltage"] for r in remote)
    assert not any("ADS1115" in r["To_Module"] and r["I2C_Address_or_voltage"]=="0x48" for r in remote)
    for main in ("README.md","Boat/README.md","Remote/README.md"):
        assert "R20" in (ROOT/main).read_text(encoding="utf-8")
    print("R20 ACTIVE MODULE-ONLY POLICY PASS (legacy R15 regression checks): two sides, "+str(len(bom))+" BOM rows, "
          +str(len(boat)+len(remote))+" uncommissioned wiring entries, 6 KiCad workflows manual-only")
    print("NO independent hardware cutoff inferred; software failsafes remain on the Boat ESP32 and physical STOP/feedback acceptance is pending")

if __name__=="__main__":
    guard()
