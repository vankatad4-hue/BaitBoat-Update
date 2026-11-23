#!/usr/bin/env python3
"""Flytec VS Code/PlatformIO entrypoint. Build/tests/source ZIP; NEVER upload.

Runs the existing endpoint-specific PlatformIO project configurations without
copying old sketch files into new compiler roots. The source ZIP includes ALL
git-tracked repo files, not merely Boat/Remote, and no untracked local secrets.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import sys
import tempfile
import zipfile

ROOT=Path(__file__).resolve().parents[1]
TARGETS={
    "boat":("Boat","boat_n32r16v_diagnostic"),
    "remote":("Remote","remote_waveshare_43_diagnostic"),
}
FIXED_ZIP_TIME=(1980,1,1,0,0,0)

def command(*args:str, cwd:Path=ROOT)->None:
    print("+"," ".join(str(a) for a in args),flush=True)
    subprocess.run([str(a) for a in args],cwd=cwd,check=True)

def git_paths(root:Path)->list[Path]:
    p=subprocess.run(["git","ls-files","-z"],cwd=root,check=True,
                     stdout=subprocess.PIPE).stdout
    result=[]
    for item in p.split(b"\0"):
        if not item:continue
        rel=Path(item.decode("utf-8","surrogateescape"))
        parsed=PurePosixPath(rel.as_posix())
        if parsed.is_absolute() or ".." in parsed.parts or ".git" in parsed.parts:
            raise ValueError(f"Unsafe tracked path: {rel}")
        full=root/rel
        if full.is_symlink():raise ValueError(f"Tracked symlink forbidden in source ZIP: {rel}")
        if not full.is_file():raise FileNotFoundError(f"Tracked source is missing: {rel}")
        result.append(rel)
    return sorted(result,key=lambda v:v.as_posix())

def snapshot(root:Path,output:Path)->tuple[int,str]:
    """Reproducible all-file Git-tracked ZIP; commit identity in manifest."""
    entries=git_paths(root)
    commit=subprocess.run(["git","rev-parse","HEAD"],cwd=root,check=True,
                          capture_output=True,text=True).stdout.strip()
    manifest={"schema":1,"purpose":"SOURCE_ONLY_NOT_SIGNED_FIRMWARE",
              "commit":commit,"file_count":len(entries),
              "endpoints":["Boat","Remote"],"build_tool":"VS Code + PlatformIO",
              "physical_acceptance":"NOT_RUN_OR_BLOCKED","files":[]}
    output.parent.mkdir(parents=True,exist_ok=True)
    if output.exists():output.unlink()
    with zipfile.ZipFile(output,"w",compression=zipfile.ZIP_DEFLATED,
                         compresslevel=6,allowZip64=True) as z:
        for rel in entries:
            data=(root/rel).read_bytes()
            name=rel.as_posix()
            manifest["files"].append({"path":name,"bytes":len(data),
                                       "sha256":hashlib.sha256(data).hexdigest()})
            info=zipfile.ZipInfo(name,date_time=FIXED_ZIP_TIME)
            info.external_attr=0o100644<<16
            info.compress_type=zipfile.ZIP_DEFLATED
            z.writestr(info,data,compress_type=zipfile.ZIP_DEFLATED,
                       compresslevel=6)
        info=zipfile.ZipInfo("FLYTEC_SOURCE_MANIFEST.json",date_time=FIXED_ZIP_TIME)
        info.external_attr=0o100644<<16
        info.compress_type=zipfile.ZIP_DEFLATED
        z.writestr(info,json.dumps(manifest,sort_keys=True,ensure_ascii=False,
                                   indent=2).encode("utf-8")+b"\n")
    digest=hashlib.sha256(output.read_bytes()).hexdigest()
    (output.parent/(output.name+".sha256")).write_text(
        f"{digest}  {output.name}\n",encoding="utf-8")
    return len(entries),digest

def audit()->None:
    command(sys.executable,"tools/verify_platformio_unified.py")
    command(sys.executable,"Boat/tools/verify_platformio_source_layout_r30h13.py")
    command(sys.executable,"Boat/tools/verify_r30h10_physical_lock.py")
    command(sys.executable,"Boat/tools/verify_r30h_radio_preflight.py")
    command(sys.executable,"-m","unittest","discover","-s","tools/tests","-p","test_*.py","-v")

def compile_host()->None:
    """Build the portable STOP fault matrices for BOTH concrete wire codecs."""
    if not shutil.which("g++"):raise RuntimeError("g++ required for host tests")
    with tempfile.TemporaryDirectory(prefix="flytec-r30h20-") as tmp:
        groups=[
            ["g++","-std=c++17","-Wall","-Wextra","-Wpedantic","-Werror",
             "-IBoat/source","Boat/tests/boat_r30h18_stop_ack_outbox_test.cpp"],
            ["g++","-std=c++17","-Wall","-Wextra","-Wpedantic","-Werror",
             "-IBoat/source","-IRemote/source",
             "Boat/source/radio_payload_codec.cpp",
             "Boat/tests/boat_r30h19_stop_ack_status_integration_test.cpp"],
            ["g++","-std=c++17","-Wall","-Wextra","-Wpedantic","-Werror",
             "-IRemote/source","Remote/source/radio_payload_codec.cpp",
             "Remote/source/radio_payload_encoder.cpp",
             "Remote/source/radio_ack_tracker.cpp",
             "Remote/tests/r30h19_ack_wire_test.cpp"],
        ]
        groups.append(["g++","-std=c++17","-Wall","-Wextra",
                       "-Wpedantic","-Werror","-IBoat/source",
                       "Boat/source/radio_payload_codec.cpp",
                       "Boat/tests/boat_r30h20_battery_status_wire_test.cpp"])
        for endpoint in ("boat","remote"):
            src="Boat" if endpoint=="boat" else "Remote"
            inc=["-Itests/r30h20_stubs",
                 "-IBoat/source","-IRemote/source"] if endpoint=="boat" else [
                 "-Itests/r30h20_stubs","-IRemote/source","-IBoat/source"]
            groups.append(["g++","-std=c++17","-Wall","-Wextra",
                           "-Wpedantic","-Werror",*inc,
                           f"{src}/source/radio_protocol.cpp",
                           f"{src}/source/radio_payload_codec.cpp",
                           "tests/r30h20_radio_frame_fault_matrix_test.cpp","-lcrypto"])
        for i,args in enumerate(groups):
            exe=str(Path(tmp)/f"host_{i}")
            command(*args,"-o",exe)
            command(exe)

def build(endpoint:str)->None:
    if not shutil.which("pio"):
        raise RuntimeError("PlatformIO CLI missing: install VS Code PlatformIO IDE")
    path,env=TARGETS[endpoint]
    command("pio","run","-d",path,"-e",env)
    print(f"{endpoint}: built only. Upload remains DENIED.")

def main()->int:
    parser=argparse.ArgumentParser(description=__doc__)
    sub=parser.add_subparsers(dest="action",required=True)
    for name in ("audit","test-host","build-boat","build-remote","build-all","snapshot"):
        p=sub.add_parser(name)
        if name=="snapshot":
            p.add_argument("--output",type=Path,
                           default=Path("out/Flytec_Project_PlatformIO_R30H20_source_snapshot.zip"))
    args=parser.parse_args()
    if args.action=="audit":audit()
    elif args.action=="test-host":compile_host()
    elif args.action=="build-boat":audit();build("boat")
    elif args.action=="build-remote":audit();build("remote")
    elif args.action=="build-all":
        audit()
        for endpoint in TARGETS:build(endpoint)
    elif args.action=="snapshot":
        output=args.output if args.output.is_absolute() else ROOT/args.output
        if not output.is_relative_to(ROOT):
            raise ValueError("Source ZIP output must stay inside the project root")
        count,digest=snapshot(ROOT,output)
        print(f"Created {output} with {count} git-tracked files; SHA-256 {digest}")
    return 0

if __name__=="__main__":
    try:sys.exit(main())
    except (RuntimeError,ValueError,FileNotFoundError,subprocess.CalledProcessError) as e:
        print(f"FLYTEC: {e}",file=sys.stderr)
        sys.exit(2)
