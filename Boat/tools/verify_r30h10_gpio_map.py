#!/usr/bin/env python3
"""R30H-10 passive GPIO source check. No physical signoff or GPIO operation."""
import csv
import io
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
B="Boat/docs/BOAT_R30H_10_PIN_MAP.csv"
R="Remote/docs/REMOTE_R30H_10_PIN_MAP.csv"
BH="Boat/source/boat_radio_hw_config.h"
RH="Remote/source/radio_hw_config.h"

def snapshot(root=ROOT):
    return {p:(root/p).read_text(encoding="utf8") for p in (B,R,BH,RH)}
def check_gpio_map(root=ROOT):
    return bool(verify(snapshot(root)))
def verify(data):
    errors=[]
    def rows(p):
        r=list(csv.DictReader(io.StringIO(data[p])))
        if len(r)!=len({x["Net"] for x in r}): errors.append(p+": duplicated net")
        return {x["Net"]:x for x in r}
    b,r=rows(B),rows(R)
    boat={"GNSS_TX":4,"GNSS_RX":5,"I2C_SCL":7,"I2C_SDA":8,"NRF_CSN":9,
          "NRF_CE":10,"NRF_MOSI":11,"NRF_SCK":12,"NRF_MISO":13,"NRF_IRQ":21,
          "E22_NRST":6,"E22_AUX":14,"E22_M0":15,"E22_M1":16,"E22_TX":17,"E22_RX":18,
          "MDD_PWM1":39,"MDD_PWM2":40,"SAFETY1":1,"SAFETY2":2,"SAFETY41":41,
          "SAFETY42":42,"GNSS_PPS":-1,"MCP23017":-1,"INA231":-1}
    remote={"I2C_SDA":8,"I2C_SCL":9,"TF_MOSI":11,"TF_SCK":12,"TF_MISO":13,
            "SD_DUMMY_CS":6,"RS485_TX":15,"RS485_RX":16,"NRF_CE":-1,
            "NRF_CSN":-1,"NRF_IRQ":-1,"REJECTED_CE":6,"REJECTED_CSN":15,
            "BUTTONS_MCP":-1,"OK_MCP":-1,"ADS1115":-1,"SC16IS750":-1,
            "USB_DN":19,"USB_DP":20,"UART0_TX":43,"UART0_RX":44,"REMOTE_PPS":-1}
    if set(b)!=set(boat): errors.append("Boat pins missing/added")
    if set(r)!=set(remote): errors.append("Remote pins missing/added")
    for m,pins in ((b,boat),(r,remote)):
        for name,pin in pins.items():
            row=m.get(name,{})
            if row.get("GPIO")!=str(pin): errors.append(name+": unexpected GPIO")
    for name in ("MDD_PWM1","MDD_PWM2"):
        if b.get(name,{}).get("Disposition")!="BLOCKED_ACTUATOR":errors.append(name+" output unlocked")
    for name in ("SAFETY1","SAFETY2","SAFETY41","SAFETY42"):
        if b.get(name,{}).get("Disposition")!="RESERVED":errors.append(name+" safety pin reused")
    for name in ("NRF_CE","NRF_CSN","NRF_IRQ","REMOTE_PPS"):
        if r.get(name,{}).get("Disposition")!="UNASSIGNED":errors.append(name+" Remote output allocated")
    for name in ("REJECTED_CE","REJECTED_CSN"):
        if r.get(name,{}).get("Disposition")!="CONFLICT_REJECTED":errors.append(name+" collision obscured")
    if b.get("GNSS_PPS",{}).get("Disposition")!="UNASSIGNED":errors.append("Boat PPS fabricated")
    for path,expected in (
       (BH,{"BOAT_NRF_CSN_PIN":9,"BOAT_NRF_CE_PIN":10,"BOAT_NRF_SCK_PIN":12,
            "BOAT_NRF_MOSI_PIN":11,"BOAT_NRF_MISO_PIN":13,"BOAT_NRF_IRQ_PIN":21,
            "BOAT_E22_UART_PIN_TX":17,"BOAT_E22_UART_PIN_RX":18,
            "BOAT_E22_UART_PIN_M0":15,"BOAT_E22_UART_PIN_M1":16,
            "BOAT_E22_UART_PIN_AUX":14,"BOAT_E22_UART_PIN_NRST":6,
            "BOAT_NRF_PIN_AUDIT_SIGNED":0,"BOAT_E22_UART_PIN_AUDIT_SIGNED":0}),
       (RH,{"REMOTE_NRF_CE_PIN":-1,"REMOTE_NRF_CSN_PIN":-1,
            "REMOTE_NRF_MOSI_PIN":11,"REMOTE_NRF_SCK_PIN":12,
            "REMOTE_NRF_MISO_PIN":13,"REMOTE_SD_DUMMY_CSN_PIN":6,
            "REMOTE_R30H10_REJECTED_CE_GPIO":6,
            "REMOTE_R30H10_REJECTED_CSN_GPIO":15,
            "REMOTE_NRF_PIN_AUDIT_SIGNED":0})):
        for key,value in expected.items():
            match=re.search(r"^#define\s+"+re.escape(key)+r"\s+\(?(-?\d+)\)?\s*$",data[path],re.M)
            if not match or int(match.group(1))!=value: errors.append(path+":"+key+" altered")
    return errors

if __name__=="__main__":
    errors=verify(snapshot())
    if errors:
        for err in errors:print("R30H-10 GPIO LOCK FAIL:",err)
        raise SystemExit(1)
    print("PASS: provisional pin maps consistent, Remote RF=-1, PPS=-1, safety and upload still locked")
    print("PASS is SOURCE ONLY; no real GPIO RF PPS or STOP measurement")
