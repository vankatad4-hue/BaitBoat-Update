# Remote Mechanical Envelope and PCB Layout R4

## Enclosure size decision
**Target external enclosure: 300 × 210 × 70 mm (W × H × depth).** This remains the baseline handheld controller envelope. This is a provisional CAD target; final dimensions must be confirmed by a printed/foam mockup with the exact screen, cells, grips and antennas.

## Front layout
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard board/display central; display active area and board outline verified from vendor drawing before CAD.
- Two joystick modules symmetrically above/at the sides of screen as user approved; 16 buttons in two banks (2 columns × 4 rows each), confirm/OK button retained.
- Power button, USB-C/service opening and optional speaker/buzzer openings must not obstruct screen or controls.
- Grips and button positions tested with hand mockup; no shoulder/rear controls.

## Internal stack
- 4× matched 18650 cells, 1S4P pack, protected/fused. Use a welded holder/pack, never loose cells; cells must be mechanically secured and insulated.
- Controller board behind screen; custom carrier board behind/adjacent to controller depending stack height and cable clearance.
- Carrier PCB target **100 × 75 mm, 4-layer, 1.6mm**, 4 mounting holes 3.2mm, 5mm edge inset (proposal only; check enclosure/board fit). High-current rail outer copper 2oz candidate, solid ground plane and separated power/RF returns.
- Component placements: nRF module toward LEFT antenna feed; E22 module toward RIGHT antenna feed; antenna coax routed separately; TPS61023 LoRa 5V converter near E22 supply branch; TPS63070 near nRF rail; SC16IS750 near E22 UART; ADS/MCP close to I/O connectors.
- The Waveshare board is ~106.8mm wide, so the proposed 100mm carrier is not intended to sit directly under the entire board as a co-planar stack. Mounting/standoff geometry and overlap must be modelled. If carrier footprint conflicts, increase/reshape it; 100×75 is only a starting proposal.

## Antenna layout
- LEFT connector: 2.4GHz nRF24 PA/LNA antenna.
- RIGHT connector: 868MHz E22 antenna.
- ESP32 Wi-Fi/Bluetooth onboard antenna remains on Waveshare board.
- Keep antenna feed/keepout away from battery cells, display shield, copper ground discontinuities and hand/grip. Test enclosure detuning and RF isolation.

## PCB and service access
- Keyed battery, display/controller, joystick, button-bank, radio and service connectors.
- Test pads: BAT_PROT, 5V_CONTROLLER, LORA_5V, RADIO_3V3, GND, I2C, SPI, UART, CE/CSN, ADC.
- Keep modules removable and connectors accessible; antenna coax must have strain relief.
- No exposed pack tabs or sharp cell holder edges; provide insulating barriers.
- Include access to USB-C, power switch, fuse and battery disconnect.

## Thermal / clearance gates
- 70mm total depth is a target, not validated. Measure screen, board, cells, joystick height, grips, coax bend and enclosure wall thickness.
- Simultaneous display+CS8501, nRF bursts and LoRa boost/module TX tests determine converter thermal placement.
- Keep heat-producing converter inductors clear of cells and plastic; ventilation is optional only if splash/water protection allows.
- Confirm waterproofing strategy: splash-resistant gasket/connector boots, but do not seal hot components without thermal review.

## Revision gate
A physical mockup and 3D assembly with vendor STEP models is required before PCB fabrication. The carrier PCB size and internal placement remain provisional.
