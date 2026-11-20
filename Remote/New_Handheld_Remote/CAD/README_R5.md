# CAD R5
Parametric shell source and cap source are provisional. Upper four right-bank buttons are dedicated hopper/hook controls. Actual component dimensions must be verified. CAD build workflow exports STLs and preview.

R5 export workflow now serializes artifact builds and rebases before committing generated STLs/preview to avoid branch-push races.

The CAD workflow now fetches and rebases the branch before pushing generated assets.

R5 CAD sources were synchronized to the generated print geometry; the build workflow exports shell, caps, preview, ZIP and SHA256.

Generated CAD assets increment the Remote package version automatically when changed.

Cap-label source fixed for OpenSCAD parser compatibility; rerun exports for this updated source.

Button-cap STL exports are text-free; identify each cap by filename and the R5 button map, avoiding OpenSCAD font-version dependencies.

Latest R5 fit geometry: maximum shell width310.1mm, height209mm; overall depth69.5mm including raised release guards. CAD STL meshes were locally checked for watertightness. These are still fit prototypes.

Automated GitHub export builds front/rear shells, the multi-cap plate and preview. Individual named release-cap STLs are included in the manual print package; the automated build intentionally avoids font/parser-dependent variants.

Assembly preview now includes display-glass and joystick placeholders to visualize the assembled handheld; these are not printed shell components.

CAD export retriggered after shared workflow concurrency fix; generated meshes and print ZIP will be versioned by the build workflow.

Queued build enabled (non-cancelling shared concurrency); latest CAD export requested after workflow race fix.

## R6 enclosure concept (2026-11-20)
- New parametric source: `Flytec_Remote_Enclosure_R6.scad`.
- Physical button banks are user-confirmed **4 columns × 2 rows each side** (8 each); left side utility functions, right side four separate guarded release controls in top row.
- R6 starting envelope 310×209mm footprint and 66.5mm shell body + 3.5mm front panel = about 70mm nominal total before antenna hardware.
- Includes front-panel apertures, rear tray/cavity, provisional mounting bosses, screen support ledges, joystick mounting rings, USB-C cutout, twin antenna bulkhead exits, vent slots and preview caps.
- This is an early geometry draft, not verified. Display/joystick/button/antenna/battery/PCB part dimensions are unconfirmed. Boss positions, cavity, screen opening, antenna alignment, hand ergonomics and printability require CAD inspection and physical fit checks.
- Automated workflow now exports R6 front, rear, button-cap plate and assembly PNG alongside the historical R5 artifacts.

### R6 CAD local validation update
- R6 panel footprint is scaled to 310×209mm; rear tray depth 66.5mm plus front plate 3.5mm, with 2mm release collars at the four guarded keys.
- Separate physical OK/confirm key is added below display; power key remains above display.
- Right-bank release keys have larger aperture and preliminary raised collar; design is not a certified physical guard.
- Local OpenSCAD export and STL mesh check succeeded: front and rear meshes are watertight single components; button plate is watertight and has 17 separate caps (12 utility, 4 release, 1 OK).
- The workflow additionally exports an orthographic R6 front-layout PNG. Fit and print validation remain pending.


## R6.1 refinement
- The R6 source now contains R6.1 internal-fit preview mode and battery-cradle export mode.
- Rear shell integrates sculpted palm pads, carrier PCB mounting posts, and a provisional cradle for four 18650 envelopes (two cells in each grip, stacked in shell depth).
- Component envelopes are placeholders only. Local exports were re-rendered; full geometry fit and live-cell safety remain unverified.
- Preview files generated locally: `Remote_Internal_Layout_R6_1.png` and `Remote_Internal_Top_R6_1.png`. GitHub workflow is updated to export both and the R6.1 battery cradle mesh.
- [R6.1 mechanical validation notes](../MECHANICAL/REMOTE_MECHANICAL_R6_1.md).
