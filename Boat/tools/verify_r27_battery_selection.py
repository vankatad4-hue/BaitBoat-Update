"""R27 static wiring/BOM verification: design only, no GPIO or sensor access."""
import csv
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
read=lambda p:(ROOT/p).read_text(encoding="utf-8")
def rows(p):
 with (ROOT/p).open(newline="",encoding="utf-8") as f:return list(csv.DictReader(f))
w=rows("Boat/docs/BOAT_R27_INA231_HARNESS_OVERRIDE.csv")
assert len(w)==10
m={x["Net"]:x for x in w}
assert len(m)==10
assert m["INA231_SCL"]["ESP_GPIO"]=="7" and m["INA231_SCL"]["ESP_Header"]=="J1-7"
assert m["INA231_SDA"]["ESP_GPIO"]=="8" and m["INA231_SDA"]["ESP_Header"]=="J1-12"
assert all(x["Design_Status"] in ("UNWIRED_PENDING","NO_WIRE") for x in w)
assert all(x["Physical_Gate"] for x in w)
assert m["INA231_BUS"]["Origin_Signal"]=="FUSED_LOW_CURRENT_SENSE"
assert m["INA231_BUS"]["Destination_Signal"]=="BUS"
assert "0.1ohm R100" in m["INA231_SHUNT_PLUS"]["Physical_Gate"]
assert m["INA231_SHUNT_PLUS"]["Design_Status"]=="NO_WIRE"
assert m["INA231_SHUNT_MINUS"]["Design_Status"]=="NO_WIRE"
b=rows("docs/PROJECT_PURCHASE_REGISTER_R20.csv")
selected=[r for r in b if r["Side"]=="Boat" and r["Item"]=="Battery voltage-only I2C sensor"]
assert len(selected)==1 and selected[0]["Qty"]=="1"
assert selected[0]["Selected_or_candidate"]=="R27_SELECTED_EXACT_ARDBOARD1491_NOT_PURCHASED"
assert "Ardboard INA231" in selected[0]["Exact_part"]
assert "INA231" in next(r["Purchase_gate"] for r in b if r["Item"]=="Sensor and I2C expansion 3V3 power")
source=read("Boat/source/ina231_voltage_contract_r27.h")
assert "INA231_MODULE_SELECTED=true" in source
assert "INA231_PHYSICAL_COMMISSIONED=false" in source
assert "readForBoatSafety" in source
firmware=read("Boat/source/tracker_sensor_port.cpp")
assert "if(boat_r27::INA231_MODULE_SELECTED)" in firmware
assert "adcPending_=false;voltageValid_=false;return;" in firmware
old=rows("Boat/docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv")
assert len([r for r in old if r["Harness_ID"]=="H03_ADS"])>=7 # historical
e=rows("Boat/docs/BOAT_R27_INA231_ACCEPTANCE.csv")
assert len(e)==21 and all(r["Status"]=="NOT_RUN" for r in e)
print("R27 pass: INA231 10-wire override, R100 no-20A, selected BOM and dormant sensor runtime; 21 acceptance rows NOT_RUN")
