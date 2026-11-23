#!/usr/bin/env python3
"""Cross-platform VS Code/PlatformIO staging for the Bulgarian LVGL 9.6 font.

Requires an existing locally licensed TTF/OTF and its separate license text.
Never copies source font bytes into the repo. Node/npm (npx) required.
This is a local developer step; release/upload gates remain unchanged.
"""
from __future__ import annotations
import argparse
import hashlib
import shutil
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--font",required=True,type=Path,help="Locally licensed TTF/OTF")
    p.add_argument("--license",required=True,type=Path,help="Text of that font license")
    a=p.parse_args()
    font=a.font.expanduser().resolve()
    license_file=a.license.expanduser().resolve()
    if not font.is_file() or font.suffix.lower() not in (".ttf",".otf"):
        p.error("--font must point to a readable local .ttf/.otf")
    if not license_file.is_file():
        p.error("--license must point to an existing text file")
    license_text=license_file.read_text(encoding="utf-8")
    if not license_text.strip():
        p.error("font license text must be non-empty")
    npx=shutil.which("npx")
    if not npx:
        p.error("Node.js/npx not installed: install Node.js 22 or newer")
    generated=ROOT/"out"/"ui-font"
    generated.mkdir(parents=True,exist_ok=True)
    target=generated/"flytec_bg_14.c"
    cmd=[npx,"--yes","--package=lv_font_conv@1.5.3","lv_font_conv",
         "--font",str(font),
         "--range","0x20-0x7E,0xB0,0xB1,0x400-0x45F,0x2013,0x2014,0x2022,0x2026,0x2190-0x2193,0x25B2",
         "--size","14","--bpp","4","--format","lvgl",
         "--no-compress","--no-prefilter","--lv-include","lvgl.h",
         "--output",str(target)]
    subprocess.run(cmd,check=True,cwd=ROOT)
    if not target.is_file() or "flytec_bg_14" not in target.read_text(encoding="utf-8"):
        raise SystemExit("LVGL font conversion did not create expected C resource")
    flag=generated/"flytec_bg_14.generated.h"
    flag.write_text("#pragma once\n#define FLYTEC_BG_FONT_GENERATED 1\n",encoding="utf-8")
    lic=generated/"FONT_LICENSE.txt"
    lic.write_text(license_text,encoding="utf-8")
    manifest=generated/"SHA256SUMS.txt"
    manifest.write_text("".join(
        hashlib.sha256(q.read_bytes()).hexdigest()+"  "+q.name+"\n"
        for q in (target,flag,lic)),encoding="utf-8")
    dest=ROOT/"Remote"/"source"
    shutil.copyfile(target,dest/target.name)
    shutil.copyfile(flag,dest/flag.name)
    # Only generated .c/.h are staged, NEVER the binary font or its source path.
    print("LVGL resource generated and staged for local PIO compile.")
    print("Source font remains local, and no repository release or hardware lock was changed.")

if __name__ == "__main__":
    main()
