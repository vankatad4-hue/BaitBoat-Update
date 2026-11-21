# R18 — Approved electronics retailers and truthful purchasing evidence
**21 November 2026. Owner rule:** Purchase only from traceable **electronics specialists, manufacturers and established electronics component distributors**. Priority is Bulgarian storefronts/companies; reputable specialist EU distributors delivering to Bulgaria can be used when verified. **AliExpress is permitted conditionally** only when exact maker identity/seller authenticity, buyer protection and actual board specification can be verified. **OLX, eMAG, generic marketplace resellers, unknown vendors and unverified clone products are excluded.**

## Accepted vendors / purchasing priority

**Tier BG — verified Bulgarian electronics storefront or Bulgarian producer**
- [Farnell Bulgaria](https://bg.farnell.com/) — official Farnell Bulgaria storefront, Bulgarian sales support `OrdersBG@farnell.com` and `info-bg@farnell.com`. Exact **RECOM R-78E3.3-0.5** is catalogued as Farnell **2218602** on the [Bulgarian manufacturer product listing](https://bg.farnell.com/c/power-supplies/dc-dc-converters/linear-regulator-replacements?brand=recom-power); 6–28V input, 3.3V/0.5A. Catalogue listing/pricing **does not prove live stock**. Prices can fluctuate and VAT/transport are excluded unless checkout explicitly states otherwise. [Official Bulgarian contact page](https://bg.farnell.com/contact-us).
- [Comet Electronics](https://store.comet.bg/) — exact RECOM part in its [RS catalogue order code 10923552](https://store.comet.bg/en/CatalogueRS/Product/10923552/): Comet Bulgaria **0 on hand** at lookup, distributor quantity displayed with **7–10 days on-request estimate**, not same-day local stock. Note a different Comet RS code **10116386** for the same MPN displayed a markedly higher price: **do not treat codes as interchangeable without getting a written quote**.
- [Olimex](https://www.olimex.com/) — electronics manufacturer in Plovdiv. [DCDC6-16-TO5](https://www.olimex.com/Products/Power-Supply/Modules/DCDC6-16-TO5/) explicitly listed **IN STOCK** on the maker's website when checked, 6–16VDC in, **5V/2A USB-A output**, barrel-jack input, 43.2×22.9mm module. Suitable **candidate** for Boat 3S-to-5V rail with a factory-made USB-A/harness adapter; not selected/commissioned for VL or Heltec yet. Check power-source output cable, regulator ripple, motor-noise filtering and approved Heltec 5V entry before connecting.
- [Ardboard](https://www.ardboard.com/) — [Heltec Wireless Tracker v1.2 exact product listing](https://www.ardboard.com/index.php?product_id=589&route=product%2Fproduct). Version named correctly; live shelf quantity and delivery at checkout **not verified**.

**Tier EU-specialist — legitimate specialist source, Bulgaria delivery not yet confirmed for exact basket**
- [Electrokit](https://www.electrokit.com/en/mosfet-drivkort-4-kan-50v-10a) (Swedish electronic specialist and EKM014 seller) — exact **EKM014 article 41024394**, selected **by owner**. Foreign specialist listing is verified; **no verified Bulgarian shop listing of that exact model** as of this check. Ask manufacturer/store for Bulgaria shipment, VAT, returns and correct revision; **do not substitute generic 4-channel relays**.
- [Lemona Electronics](https://www.lemona-electronics.eu/en/search.html?query=Cytron+MDD20A) (electronics specialist in EU) — exact **Cytron MDD20A** listed (search shows CTN-18881), Bulgarian checkout/shipping **not confirmed**. Existing selection unchanged.
- [Cytron manufacturer](https://www.cytron.io/c-industry/p-20amp-6v-30v-dc-motor-driver-2-channels) — exact manufacturer's product, useful as authentic source and specification baseline; not a Bulgarian local listing.
- [TME](https://www.tme.eu/), [Mouser Bulgaria](https://www.mouser.bg/), [DigiKey Bulgaria](https://www.digikey.bg/) — established electronics distributors to check as needed; these are **approved supplier categories**, **not proof of an exact part listing/stock**. Farnell and Comet may also handle official special-order quotations.

**Tier conditional marketplace**
- **AliExpress only as last-resort opt-in**: no AliExpress listing is currently approved or confirmed. Accept **only** a verifiable manufacturer-operated/authorized storefront, exact full manufacturer model and PCB revision, credible traceable warranty and invoices, real datasheet/pinout and sane price, factory photos, shipping/returns to Bulgaria and independent physical test before use. A high star rating or seller's use of an OEM logo is **not** proof of authenticity, especially for safety relays, battery BMS and high-current FET drivers. For independent STOP, contactor and timer safety modules, **prefer an authorized industrial distributor, not AliExpress**.

## Evidence definitions (these MUST NOT be conflated)
- **EXACT_LISTED_BG** = exact MPN at a genuine Bulgarian electronics shop/distributor; stock may be unavailable.
- **MFR_IN_STOCK_BG** = directly in-stock message on a Bulgaria-based manufacturer website at the time checked; recheck before purchase.
- **BG_DISTRIBUTOR_ORDER_ONLY** = no local stock reported, supplier/distributor lead time indicated (not guaranteed).
- **EXACT_LISTED_EU_SHIP_UNVERIFIED** = exact part at a verified specialist outside Bulgaria; Bulgarian shipping/quantity/tax unverified.
- **NO_APPROVED_BG_LISTING** = not found at approved Bulgarian shops in this limited search; it is **not proof the part cannot be sourced**.
- **NO_VERIFIED_LISTING** = cannot assign supplier for this exact module; prohibit inventing stock or seller identity.
- **DO_NOT_BUY_UNQUALIFIED** = critical function not technically approved, irrespective of stock.

## Procurement gates
Before adding an item to a real order: check manufacturer model **including revisions**, vendor SKU, rated battery Vmin/Vmax and pulse/continuous currents, REAL IN STOCK/lead time at checkout, VAT/freight to Bulgaria, invoice/warranty, environmental and exact connectors. For EKM014 inspect the physical mounting hole discrepancy **27×25mm vs 41×25mm listed**; this is a drawing/board-revision question, **not** an exclusion due to the old 77×79 carrier.

**No purchase has been made through this chat. No live quantities are reserved.** All independent hardware STOP, no-repeat/one-hot pulse, current-zero and brownout functions remain unselected; do not energize motors or solenoids. Remote VER028 and Boat VER014 unchanged; sonar still deferred.
