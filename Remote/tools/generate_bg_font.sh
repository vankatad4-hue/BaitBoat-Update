#!/usr/bin/env bash
set -euo pipefail
# Build-time resource only. Do not overwrite a source font or system font.
font="${1:-/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf}"
out="${2:-out/ui-font}"
license="${3:-/usr/share/doc/fonts-dejavu-core/copyright}"
test -r "$font"
test -r "$license"
mkdir -p "$out"
npx --yes --package=lv_font_conv@1.5.3 lv_font_conv \
  --font "$font" --range '0x20-0x7E,0xB0,0xB1,0x400-0x45F,0x2013,0x2014,0x2022,0x2026,0x2190-0x2193,0x25B2' \
  --size 14 --bpp 4 --format lvgl --no-compress --no-prefilter --lv-include lvgl.h \
  --output "$out/flytec_bg_14.c"
test -s "$out/flytec_bg_14.c"
grep -Eq 'lv_font_t[[:space:]]+flytec_bg_14' "$out/flytec_bg_14.c"
cp "$license" "$out/FONT_LICENSE.txt"
(cd "$out" && sha256sum flytec_bg_14.c FONT_LICENSE.txt > SHA256SUMS.txt)
