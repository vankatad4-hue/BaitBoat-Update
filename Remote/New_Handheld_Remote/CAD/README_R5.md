# CAD R5
Parametric shell source and cap source are provisional. Upper four right-bank buttons are dedicated hopper/hook controls. Actual component dimensions must be verified. CAD build workflow exports STLs and preview.

R5 export workflow now serializes artifact builds and rebases before committing generated STLs/preview to avoid branch-push races.

The CAD workflow now fetches and rebases the branch before pushing generated assets.
