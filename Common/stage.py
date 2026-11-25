#!/usr/bin/env python3
"""Materialize canonical Common/source into an endpoint's existing build root.
Generated copies are disposable, but local edits/collisions fail closed.
No radio or actuator initialization; standard-library Python only.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import tempfile

ROOT = Path(__file__).resolve().parent.parent
SIDES = ('Boat', 'Remote')
STAMP = '.common-staged.json'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def atomic_write(path, data):
    fd, name = tempfile.mkstemp(prefix='.common-', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as out:
            out.write(data)
        os.replace(name, path)
    finally:
        if os.path.exists(name):
            os.unlink(name)


def valid_name(name):
    return isinstance(name, str) and Path(name).name == name and '/' not in name and '\\' not in name and Path(name).suffix in ('.h', '.cpp', '.c')


def stage(root=ROOT, sides=None, check=False):
    root = Path(root).resolve()
    common = root / 'Common/source'
    if not common.is_dir() or common.is_symlink():
        raise RuntimeError('Missing canonical Common/source; extract the entire project ZIP')
    canonical = {}
    for p in sorted(common.iterdir()):
        if not valid_name(p.name) or not p.is_file() or p.is_symlink():
            raise RuntimeError(f'Unexpected canonical source: {p}')
        canonical[p.name] = p.read_bytes()
    if not canonical:
        raise RuntimeError('Empty canonical source')
    expected = {n: digest(data) for n, data in canonical.items()}
    selected = tuple(sides) if sides is not None else tuple(s for s in SIDES if (root/s/'platformio.ini').is_file())
    if not selected or any(s not in SIDES for s in selected):
        raise RuntimeError('No valid Flytec endpoint selected')
    # Preflight every endpoint before modifying any file.
    plans = []
    for side in selected:
        dest = root / side / 'source'
        if not dest.is_dir() or dest.is_symlink():
            raise RuntimeError(f'Missing endpoint source: {dest}')
        stamp = dest / STAMP
        if stamp.is_symlink():
            raise RuntimeError(f'Unsafe generated manifest: {stamp}')
        previous = json.loads(stamp.read_text(encoding='utf-8')) if stamp.exists() else {}
        if not isinstance(previous, dict) or any(not valid_name(n) or not isinstance(h,str) or len(h)!=64 for n,h in previous.items()):
            raise RuntimeError(f'Invalid generated manifest: {stamp}')
        writes, deletes = [], []
        for n, h in expected.items():
            p = dest / n
            if p.is_symlink() or (p.exists() and not p.is_file()):
                raise RuntimeError(f'Unsafe staging path: {p}')
            if p.exists():
                actual = digest(p.read_bytes())
                if actual == h:
                    continue
                if actual != previous.get(n):
                    raise RuntimeError(f'Local edit or source collision: {p}; edit Common/source instead')
            writes.append(n)
        for n in previous.keys() - expected.keys():
            p = dest / n
            if p.is_symlink() or (p.exists() and (not p.is_file() or digest(p.read_bytes()) != previous[n])):
                raise RuntimeError(f'Locally edited obsolete generated source: {p}')
            if p.exists():
                deletes.append(n)
        if check and (writes or deletes or previous != expected):
            raise RuntimeError(f'{side} common source is not current; run python Common/stage.py')
        plans.append((dest, stamp, writes, deletes, previous))
    if not check:
        for dest, stamp, writes, deletes, previous in plans:
            for n in writes:
                atomic_write(dest/n, canonical[n])
            for n in deletes:
                (dest/n).unlink()
            if previous != expected or not stamp.exists():
                atomic_write(stamp, (json.dumps(expected, sort_keys=True, indent=2)+'\n').encode('utf-8'))
    print(f'Common/source: {len(expected)} canonical files; {", ".join(selected)} {"verified" if check else "staged"}')
    return expected


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--side', choices=SIDES, action='append')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    stage(ROOT, args.side, args.check)


# SCons/PlatformIO pre-script; invoke before build source scanning.
if 'Import' in globals():
    Import('env')  # type: ignore[name-defined]
    project = Path(env.subst('$PROJECT_DIR')).resolve()  # type: ignore[name-defined]
    if project.parent != ROOT or project.name not in SIDES:
        raise RuntimeError('Open the extracted Boat or Remote PlatformIO project with Common alongside')
    stage(ROOT, [project.name])
elif __name__ == '__main__':
    main()
