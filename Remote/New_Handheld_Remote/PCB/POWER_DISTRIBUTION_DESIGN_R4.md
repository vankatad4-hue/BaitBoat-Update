# Remote Power Distribution Design R4

**Status:** Proposed architecture revision. Onboard controller boost is baseline; external boost removed pending bench validation.

## Battery
Protected 1S4P pack. BMS required. Candidate 4A fast-acting fuse, FDS6575 P-MOS reverse protection (Drain pack input, Source protected output), battery ADC/NTC. Exact parts and tests pending.

## Controller/display power
Use Waveshare ESP32-S3-Touch-LCD-4.3 Standard board's PH2.0 single-cell battery input. Its onboard CS8501 circuit boosts one cell to 5V; manufacturer board consumption is 5V/450mA. External TPS61023 is not baseline. Validate onboard boost at startup, maximum backlight, Wi-Fi and low pack voltage; external boost is fallback if needed, never parallel with onboard output.

## Radio 3.3V rail
Dedicated TPS63070 buck-boost from BAT_PROT; 1.5µH inductor candidate; 470k/150k feedback, nominal 3.3067V; input/output capacitors per component review; VAUX 100nF. EN controlled after MCU boot (default OFF). Test PA/LNA TX burst load up to 300mA.

## Current estimate @3V
Onboard controller 5V/450mA at assumed 90% ≈0.83A battery; radio 3.3V/300mA at assumed 90% ≈0.37A. Combined ≈1.20A continuous. Estimate only; onboard converter efficiency and transient capacity unverified. Fuse/BMS/wiring coordination pending.

## Layout and gates
Protection path, radio converter loops, RF keepout, testpoints and ADC/NTC. Verify board battery connector/polarity, CS8501 performance, TPS63070 pins/caps, protection parts, schematic ERC, review, PCB and bench tests. Charging is outside scope.
