# New Handheld Remote Mechanical Envelope R1

## Recommended revised enclosure envelope
The former concept was too tight to place a 4.3-inch display, two full-size joystick modules, two 2×4 button banks, four 18650 cells, radio modules, coax bulkhead exits, and comfortable grips without stacking components dangerously.

**Proposed outer size: 300 mm wide × 205 mm high × 68 mm maximum depth.** This is a design envelope, not measured CAD. Use a two-piece shell with internal ribs and a removable electronics/battery cradle.

## Front layout
- Screen centered: board/display assembly envelope roughly 107 × 68 mm (confirm exact touch board revision).
- Two joystick modules in upper left/right zones, centered approximately 55–65mm out from centerline; stick cap travel and hand clearance must be checked with actual joystick part.
- 16 buttons in symmetrical 2×4 banks, one bank each side/below joysticks. Target cap diameter 12–16mm, center spacing ≥18mm (subject to chosen switches).
- Power/confirm controls placed clear of joystick throw and button banks.
- No shoulder/rear controls. Rear grips are passive ergonomics only.
- Antenna exits on upper rear edge: LEFT 2.4GHz nRF24; RIGHT 868MHz LoRa. Keep antennas/cables from hand and metal inserts.

## Rear layout / stack
- Four matched 18650 cells in 1S4P cradle, placed across lower center/bottom, cells individually insulated and mechanically restrained; do not rely on foam alone.
- 1S4P protection/BMS board and fuse at battery-positive entry, accessible for service.
- Controller board mounted on standoffs; carrier PCB adjacent or beneath only if clearances, RF and thermal checks permit. Do not place radio modules under battery cells.
- LoRa module near right-side antenna coax exit; nRF PA/LNA module near left-side coax exit.
- Bulkhead coax connectors, with strain relief; avoid sharp coax bends.
- USB-C access for controller board programming/charging; do not connect an unverified external charger to the pack.

## Fit constraints
- 18650 typical envelope 18.4mm diameter × 65mm length; actual cell brand/holder adds tolerance. Four cells in a row need ~76mm width plus cradle/insulation; 1S4P topology wiring must be low-resistance and matched.
- Joystick travel: minimum 20mm clearance to shell/adjacent controls in every direction; exact module height and stick shaft are to be measured.
- Buttons: verify cap height, travel and switch body height; reserve ≥3mm internal clearance over switch actuators.
- Waveshare display board behind panel requires connector bend radius and touch/display flex protection; do not pinch FPC.
- Provide service loops for antenna pigtails and detachable harnesses.
- Waterproofing target: splash-resistant, not submersible; design gasket lip and sealed USB/antenna interfaces, but don't claim IP rating without test.

## CAD checkpoints
1. Insert exact board, joystick, switch, cell, antenna connector and module models.
2. Build 3D clearance model with grips and internal mounting.
3. Print front/rear fit-check prototype in low-cost material.
4. Test screen visibility in daylight, joystick reach, button reach, one-handed grip and antenna orientation.
5. Revise envelope based on physical mockup; then freeze shell and PCB placement.
