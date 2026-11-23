#!/usr/bin/env python3
"""R30H-20: enforce the actual two-root VS Code+PlatformIO compile contract.
Read-only. Never initialize I2C, radio, motors, solenoids, or an upload action.
"""
from __future__ import annotations
import configparser
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
BRANCH="platformio-unified-r30h20"
TARGETS={
 "Boat":("boat_n32r16v_diagnostic","opi_opi","32MB",False),
 "Remote":("remote_waveshare_43_diagnostic","qio_opi","16MB",True),
}
WORKFLOWS=[
 ".github/workflows/platformio-migration-build.yml",
 ".github/workflows/boat-host-tests.yml",
 ".github/workflows/remote-host-tests.yml",
 ".github/workflows/source-audit.yml",
 ".github/workflows/package-flytec-zips.yml",
]
def verify(root:Path=ROOT)->list[str]:
    errors=[]
    for side,(env,memory,flash,ino) in TARGETS.items():
        path=root/side/"platformio.ini"
        conf=configparser.ConfigParser(interpolation=None)
        try:conf.read(path,encoding="utf-8")
        except (OSError,configparser.Error) as exc:
            errors.append(f"{side} PlatformIO config error: {exc}");continue
        if not conf.has_section("platformio") or not conf.has_section("env:"+env):
            errors.append(f"{side} missing default environment {env}");continue
        pio=conf["platformio"];target=conf["env:"+env]
        required={
            "default_envs":(pio.get("default_envs"),env),
            "src_dir":(pio.get("src_dir"),"source"),
            "platform":(target.get("platform"),"espressif32 @ 8.0.0"),
            "framework":(target.get("framework"),"arduino"),
            "board":(target.get("board"),"esp32-s3-devkitc-1"),
            "memory_type":(target.get("board_build.arduino.memory_type"),memory),
            "flash_size":(target.get("board_upload.flash_size"),flash),
            "upload_protocol":(target.get("upload_protocol"),"custom"),
        }
        for field,(found,expected) in required.items():
            if found!=expected:errors.append(f"{side} {field}: expected {expected!r}, got {found!r}")
        if "pio_deny_upload.py" not in target.get("upload_command",""):
            errors.append(f"{side} upload is not explicitly blocked")
        filt=target.get("build_src_filter","")
        if "+<*.cpp>" not in filt or ("+<*.ino>" if ino else "-<*.ino>") not in filt:
            errors.append(f"{side} source filter regression")
        if "BOARD_HAS_PSRAM" not in target.get("build_flags",""):
            errors.append(f"{side} PSRAM source configuration missing")
        if not (root/side/"tools/pio_deny_upload.py").exists():
            errors.append(f"{side} missing deny-upload script")
        if not (root/side/"source").is_dir():
            errors.append(f"{side} missing production compiler root")
    try:
        ws=json.loads((root/"Flytec_V080.code-workspace").read_text(encoding="utf-8"))
        folders={v["path"] for v in ws["folders"]}
        if not {"Boat","Remote"}.issubset(folders):errors.append("VS Code workspace missing Boat/Remote roots")
        labels={v["label"] for v in ws["tasks"]["tasks"]}
        if not {"Flytec: Boat PlatformIO build (NO UPLOAD)",
                "Flytec: Remote PlatformIO build (NO UPLOAD)"}.issubset(labels):
            errors.append("VS Code workspace missing separate build tasks")
    except (OSError,ValueError,KeyError) as exc:errors.append(f"VS Code workspace invalid: {exc}")
    for rel in WORKFLOWS:
        try:
            s=(root/rel).read_text(encoding="utf-8")
            if BRANCH not in s:errors.append(f"{rel} does not watch {BRANCH}")
        except OSError as exc:errors.append(f"{rel} missing: {exc}")
    for side,expected in (("Boat","014"),("Remote","028")):
        path=root/side/"releases/VERSION"
        try:
            got=path.read_text(encoding="utf-8").strip()
            if got!=expected:errors.append(f"{side} official release changed to {got}")
        except OSError as exc:errors.append(str(exc))
    if errors:return errors
    print("Flytec R30H-20: dual-root PlatformIO/VS Code, upload locks, version freeze PASS (source only)")
    return []
if __name__=="__main__":
    failures=verify()
    for issue in failures:print("FAIL:",issue)
    raise SystemExit(1 if failures else 0)
