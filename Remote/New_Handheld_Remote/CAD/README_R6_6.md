# Remote R6 CAD source and print package

- `Flytec_Remote_Enclosure_R6.scad`: parametric source.
- Front: 2×4 key banks on each side, single power key above screen, OK bottom-right of right bank, USB-C and dual antenna exits.
- OK is a physical key within the bank but has a separate MCP23017 0x21 GPA4 input. Fifteen other keys are scanned on MCP23017 0x20.
- Four right-bank top keys have preliminary guard collars and are independent hopper-left/right and hook-left/right controls.
- `part` modes: front, rear, buttons, assembly, internal.
- Button cap plate: 11 utility caps + 4 release caps + 1 OK cap.

**Not manufacturing-ready:** actual board/switch/joystick/battery/module dimensions, connector access, PCB mounting, RF clearance, battery restraints, charging/thermal design, CAD fit, tolerance, print orientation and STL export remain unvalidated. The corrected SCAD source is committed; front/rear/button meshes were locally checked using a reconstructed mirror; repository workflow exports and assembly/internal previews remain pending verification.
