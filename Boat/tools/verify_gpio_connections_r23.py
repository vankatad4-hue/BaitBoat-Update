"""R23 statically verify the active GPIO/header + module-only wiring contract.

This checks design consistency only; it does not approve a physical PCB or
physical safety cutoffs. No hardware is energized.
"""
from pathlib import Path
import csv
import re

BOAT=Path(__file__).resolve().parents[1]
R20=BOAT/"docs/BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv"
R23=BOAT/"docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv"
CONTRACT=BOAT/"source/boat_gpio_connection_contract.h"
RADIO=BOAT/"source/boat_radio_hw_config.h"
read=lambda p:p.read_text(encoding="utf-8")

rows=list(csv.DictReader(R23.open(encoding="utf-8",newline="")))
old=list(csv.DictReader(R20.open(encoding="utf-8",newline="")))
assert rows and len(rows)>=60, "R23 incomplete module net matrix"
assert len({r["Net"] for r in rows})==len(rows), "Duplicate R23 net"
headers={f'{r["Header"]}-{r["Pin"]}':r for r in old}
pins={}
for r in rows:
    p=r["ESP_GPIO"]
    h=r["ESP_Header"]
    if bool(p)!=bool(h):
        raise AssertionError(f'{r["Net"]}: GPIO/header fields must be paired')
    if not p:continue
    expect=headers.get(h)
    if expect is None or expect["Signal"]!="GPIO"+p:
        raise AssertionError(f'{r["Net"]}: GPIO{p} does not match R20/Espressif header {h}')
    if r["Design_Status"]=="OUTPUT_LOCKED" and r["Direction"]=="ESP_TO_MODULE":
        pass  # assignments exist but outputs must remain disabled
    if r["Design_Status"]=="RESERVED_NO_WIRE" and r["Destination"]!="UNSELECTED_SAFETY_FEEDBACK":
        raise AssertionError("Unsafe reserved safety wiring")
    if p in {"0","3","19","20","35","36","37","38","43","44","45","46","47","48"}:
        raise AssertionError("Strapping USB memory RGB or 1V8 pin was assigned: "+p)
    pins.setdefault(p,set()).add(r["Net"])
used_cores={p for p in pins if p not in {"1","2","41","42"}}
assert used_cores=={"4","5","6","7","8","9","10","11","12","13",
                   "14","15","16","17","18","21","39","40"},used_cores
assert len([r for r in rows if r["Design_Status"]=="RESERVED_NO_WIRE"])==4
for p in ("1","2","41","42"):
    assert pins[p]=={"SAFETY_SPARE_GPIO"+p},p
assert not any(r["Design_Status"] in {"APPROVED","COMMISSIONED","ACTUATOR_ACTIVE"} for r in rows)
assert all(r["Physical_Verification_Gate"].strip() for r in rows), "Missing physical gates"
assert all(r["Design_Status"]=="OUTPUT_LOCKED" for r in rows
           if r["Harness_ID"] in {"H07_MDD","H08_EKM"} and
           (r["Net"].startswith("MDD_PWM") or r["Net"].startswith("MDD_DIR") or
            r["Net"].startswith("MDD_MOTOR") or r["Net"].startswith("EKM_IN") or
            r["Net"].startswith("EKM_LOAD") or r["Net"]=="EKM_VH"))
assert {r["Net"] for r in rows if r["Source"]=="ESP32-S3" and r["ESP_GPIO"] in {"7","8"}} >= {
    "M100_QMC_SCL","M100_QMC_SDA","GY91_SCL","GY91_SDA",
    "ADS_SCL","ADS_SDA","MCP_SCL","MCP_SDA"
}
assert {r["Net"] for r in rows if r["Net"].startswith("MCP_A") and "_HIGH" in r["Net"]}=={
    "MCP_A0_HIGH","MCP_A1_HIGH","MCP_A2_HIGH"}
assert {r["Net"] for r in rows if r["Net"].startswith("EKM_IN")}=={
    "EKM_IN1","EKM_IN2","EKM_IN3","EKM_IN4"}
assert {r["Net"] for r in rows if r["Net"].startswith("E22_")} >={
    "E22_NRST","E22_M0","E22_M1","E22_UART_TX_FROM_ESP",
    "E22_UART_RX_TO_ESP","E22_AUX","E22_POWER","E22_GROUND","E22_ANTENNA"}
assert "UNCOMMISSIONED" in read(BOAT/"hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino")
text=read(CONTRACT)
pairs=re.findall(r'\{"([A-Z0-9_]+)",(\d+),"((?:J1|J3)-\d+)"\}',text)
assert len(pairs)==18, "R23 C++ contract expected 18 external MCU nets"
assert len({n for n,p,h in pairs})==len(pairs)
assert len({p for n,p,h in pairs})==len(pairs)
for n,p,h in pairs:
    assert h in headers and headers[h]["Signal"]=="GPIO"+p,(n,p,h)
    assert any(r["ESP_GPIO"]==p and r["ESP_Header"]==h for r in rows),n
assert "PHYSICAL_GPIO_AUDIT_SIGNED=false" in text
assert "ACTUATOR_OUTPUTS_COMMISSIONED=false" in text
assert "SENSOR_POWER_SEQUENCE_ACCEPTED=false" in text
radio=read(RADIO)
for macro,value in [
    ("BOAT_NRF_CE_PIN",10),("BOAT_NRF_CSN_PIN",9),
    ("BOAT_NRF_SCK_PIN",12),("BOAT_NRF_MOSI_PIN",11),
    ("BOAT_NRF_MISO_PIN",13),("BOAT_NRF_IRQ_PIN",-1),
    ("BOAT_OPEN_ECHO_UART_RX_PIN",21),
    ("BOAT_E22_UART_PIN_TX",17),("BOAT_E22_UART_PIN_RX",18),
    ("BOAT_E22_UART_PIN_M0",15),("BOAT_E22_UART_PIN_M1",16),
    ("BOAT_E22_UART_PIN_AUX",14),("BOAT_E22_UART_PIN_NRST",6),
]:
    assert re.search(r'^#define\s+'+macro+r'\s+'+str(value)+r'\s*$',radio,re.M),macro
for macro in ("BOAT_NRF_PIN_AUDIT_SIGNED","BOAT_OPEN_ECHO_UART_PIN_AUDIT_SIGNED",
              "BOAT_E22_UART_PIN_AUDIT_SIGNED",
              "BOAT_E22_UART_LOGIC_LEVELS_VERIFIED",
              "BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED"):
    assert re.search(r'^#define\s+'+macro+r'\s+0\s*$',radio,re.M),macro
assert "c.pinAuditSigned=false" in read(BOAT/"source/tracker_sensor_profile.example.h")
print(f"R23 pass: {len(rows)} wiring nets and 18 independent GPIOs match DevKitC J1/J3; outputs locked; physical approvals false")
