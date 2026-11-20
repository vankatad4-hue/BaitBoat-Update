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
