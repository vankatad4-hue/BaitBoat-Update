#!/usr/bin/env python3
"""R18 supplier authenticity and distributed-mounting document guard.

Repository consistency only. It cannot check live warehouse inventory,
AliExpress seller legitimacy, or an installed boat's real electrical safety.
"""
from pathlib import Path
from urllib.parse import urlparse
import csv

ROOT=Path(__file__).resolve().parents[2]
REGISTER=ROOT/"docs/VERIFIED_ELECTRONICS_SOURCING_R18.csv"
POLICY=ROOT/"docs/APPROVED_ELECTRONICS_SUPPLIERS_R18.md"
LAYOUT=ROOT/"Boat/docs/BOAT_DISTRIBUTED_MODULE_INSTALLATION_R18.md"
BOM=ROOT/"docs/MODULE_ONLY_BOM_R15.csv"

ALLOWED_DOMAINS={
    "bg.farnell.com","store.comet.bg","www.olimex.com","www.ardboard.com",
    "www.electrokit.com","docs.electrokit.com","www.lemona-electronics.eu",
    "www.cytron.io","heltec.org","recom-power.com","www.recom-power.com",
    "duelco.com","www.albrightinternational.com","www.waveshare.com",
    "www.adafruit.com",
}
BAD_DOMAINS={"olx.bg","www.olx.bg","emag.bg","www.emag.bg",
             "aliexpress.com","www.aliexpress.com","amazon.com","www.amazon.com"}
BG_TIERS={"BG_ELECTRONICS","BG_DISTRIBUTOR","BG_MANUFACTURER"}
BG_STOCK_PROOF="MFR_IN_STOCK_BG"
VALID_STATUSES={
    "EXACT_LISTED_BG",
    "MFR_IN_STOCK_BG",
    "BG_DISTRIBUTOR_ORDER_ONLY",
    "EXACT_LISTED_EU_SHIP_UNVERIFIED",
    "MANUFACTURER_LISTED_SHIP_UNVERIFIED",
    "NO_VERIFIED_LISTING",
    "SEARCH_CANDIDATE_UNVERIFIED_EXACT",
}

def read_csv(path):
    with path.open(newline="",encoding="utf-8") as f:
        return list(csv.DictReader(f))

def domain(url):
    if not url:
        return None
    assert url.startswith("https://"),f"Non-HTTPS supplier link {url!r}"
    return urlparse(url).hostname

def check():
    s=POLICY.read_text(encoding="utf-8")
    m=LAYOUT.read_text(encoding="utf-8")
    assert "Farnell Bulgaria" in s and "Comet Electronics" in s
    assert "Olimex" in s and "Ardboard" in s and "AliExpress" in s
    assert "eMAG" in s and "OLX" in s
    assert "RETIRED AS HARD LIMITS" in m
    assert "distributed" in m.lower()
    assert "No custom PCB" in m
    assert "Duelco" in m and "Albright" in m and "EKM014" in m
    assert "77 × 79 × 14mm" in m
    assert "30mm lid clearance" in m

    items=read_csv(REGISTER)
    assert len(items)>=15, len(items)
    assert len({(r["Component"],r["Supplier"]) for r in items})==len(items)
    assert any(r["Supplier"]=="Farnell Bulgaria" and
               r["Exact_Manufacturer_Part"]=="RECOM R-78E3.3-0.5" and
               r["Evidence_Status"]=="EXACT_LISTED_BG" and
               "bg.farnell.com" in r["Supplier_URL"] for r in items)
    assert any(r["Supplier"]=="Comet Electronics" and
               r["Exact_Manufacturer_Part"]=="RECOM R-78E3.3-0.5" and
               r["Evidence_Status"]=="BG_DISTRIBUTOR_ORDER_ONLY" and
               "10923552" in r["Supplier_URL"] for r in items)
    assert any(r["Supplier"]=="Olimex" and
               r["Exact_Manufacturer_Part"]=="Olimex DCDC6-16-TO5" and
               r["Evidence_Status"]==BG_STOCK_PROOF for r in items)
    assert any(r["Supplier"]=="Ardboard" and
               r["Exact_Manufacturer_Part"]=="Heltec Wireless Tracker v1.2" for r in items)
    for target in ("Electrokit EKM014 41024394","Cytron MDD20A"):
        exact=[r for r in items if r["Exact_Manufacturer_Part"]==target]
        assert exact and all(r["Supplier_Tier"] not in BG_TIERS for r in exact)
        assert any(r["Evidence_Status"]=="EXACT_LISTED_EU_SHIP_UNVERIFIED" for r in exact)
    for r in items:
        assert r["Last_Check"]=="2026-11-21"
        assert r["Evidence_Status"] in VALID_STATUSES
        for key in ("Supplier_URL","Manufacturer_or_Spec_URL"):
            d=domain(r[key])
            if d:
                assert d in ALLOWED_DOMAINS,("Unverified shop or manufacturer",r["Component"],d)
                assert d not in BAD_DOMAINS
        if r["Supplier_Tier"] in BG_TIERS:
            assert r["Supplier_URL"],f"BG listing without URL: {r['Component']}"
            assert r["Evidence_Status"] in ("EXACT_LISTED_BG","BG_DISTRIBUTOR_ORDER_ONLY",BG_STOCK_PROOF,"SEARCH_CANDIDATE_UNVERIFIED_EXACT")
        if r["Evidence_Status"]==BG_STOCK_PROOF:
            assert r["Supplier"]=="Olimex" and r["Supplier_Tier"]=="BG_MANUFACTURER"
        if r["Supplier"]=="AliExpress":
            assert r["Evidence_Status"]=="NO_VERIFIED_LISTING"
            assert not r["Supplier_URL"] and r["Bulgaria_Delivery"]=="NOT_APPROVED"
        if r["Evidence_Status"] in ("NO_VERIFIED_LISTING","SEARCH_CANDIDATE_UNVERIFIED_EXACT"):
            assert r["Bulgaria_Delivery"] not in ("LOCAL_IN_STOCK","BG_IN_STOCK")
    bom=read_csv(BOM)
    selected=[r for r in bom if r["Side"]=="Boat" and r["Part_Number"]=="EKM014"]
    assert len(selected)==1 and selected[0]["State"]=="OWNER_SELECTED_NOT_INSTALLED_NOT_QUALIFIED"
    for x in ("docs/CONTINUATION_CHECKPOINT_VER001.md",
              "Boat/docs/PROJECT_MASTER_STATUS.md",
              "Remote/releases/CHANGELOG_MASTER_VER026.md"):
        assert "R18" in (ROOT/x).read_text(encoding="utf-8")
    print(f"R18 PASS: {len(items)} supplier entries, BG/foreign distinctions and approved retailer URLs checked")
    print("Farnell BG RECOM and Bulgarian Olimex source verified as listings; exact EKM014 / MDD20A BG seller NOT proven")
    print("NO live prices reserved, no AliExpress seller approved, and NO actuator power commissioned")

if __name__=="__main__":
    check()
