# CAD R5
Parametric shell source and cap source are provisional. Upper four right-bank buttons are dedicated hopper/hook controls. Actual component dimensions must be verified. CAD build workflow exports STLs and preview.

R5 export workflow now serializes artifact builds and rebases before committing generated STLs/preview to avoid branch-push races.

The CAD workflow now fetches and rebases the branch before pushing generated assets.

R5 CAD sources were synchronized to the generated print geometry; the build workflow exports shell, caps, preview, ZIP and SHA256.

Generated CAD assets increment the Remote package version automatically when changed.

Cap-label source fixed for OpenSCAD parser compatibility; rerun exports for this updated source.

Button-cap STL exports are text-free; identify each cap by filename and the R5 button map, avoiding OpenSCAD font-version dependencies.
