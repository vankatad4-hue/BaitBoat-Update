# Remote Power Schematic Capture Specification R2

Guide for native KiCad capture; not an actual schematic, ERC report or fabrication output.

Sheet 1 Battery: BMS protected PACK+→F_MAIN 4A Littelfuse 0458004.DR fast-acting candidate→Q_REV FDS6575 Drain; Source→BAT_PROT; Gate→100k→GND. Verify pin numbering and body diode orientation. PACK−→GND. ADC divider 220k BAT_PROT→ADC_BAT, 68k ADC→GND, 100nF. NTC 10k B3950 candidate to GND, 10k pull-up from controller 3V3, 100nF; open/short detect. Testpoints.

Sheet 2 5V: TPS61023DRLR, BAT_PROT→VIN; 1µH BAT_PROT→SW; CIN 10µF +100nF; VOUT→+5V_MAIN; COUT 2×22µF; FB 732k top/100k bottom; EN default ON via 100k BAT_PROT pullup candidate. Verify exact pin map, cap bias and layout. 450mA continuous/1A transient target, thermal pending.

Sheet 3 Radio: TPS63070, BAT_PROT→VIN; 1.5µH between L1/L2; divider 470k/150k; cap banks per calculations; VAUX 100nF. EN MCU-controlled after boot, 1k series candidate/100k pulldown; default OFF. Use pin map R3. Connector with radio 3V3, GND, SPI and control; local 100nF+10–47µF.

Sheet 4 Controller: +5V_MAIN feeds Waveshare documented input. BOOST_EN independent of MCU; RADIO_EN reset-safe. Testpoints on all rails/sense.

Layout: compact switching loops, minimal SW copper, quiet FB/ADC, RF keepout, common ground, adequate power copper. Capture gates: verified library symbols/pins, connector pinouts, ERC, independent review, PCB/DFM, bench tests.
