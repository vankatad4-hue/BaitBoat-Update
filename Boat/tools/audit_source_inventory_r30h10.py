#!/usr/bin/env python3
"""Conservative read-only Boat/Remote source inventory, not a hardware acceptance."""
import argparse
import hashlib
import json
import pathlib
import re
from collections import defaultdict

ROOT = pathlib.Path(__file__).resolve().parents[2]
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)
EXT = {'.h', '.cpp', '.c', '.ino'}

def rel(p):
    return p.relative_to(ROOT).as_posix()

def inventory():
    return sorted(p for end in ('Boat', 'Remote')
                  for p in (ROOT / end / 'source').rglob('*')
                  if p.is_file() and p.suffix in EXT)

def build_roots():
    boat = ROOT / 'Boat/source'
    remote = ROOT / 'Remote/source'
    return sorted(list(boat.glob('*.cpp')) +
                  list(remote.glob('*.cpp')) +
                  [boat / 'firmware/Flytec_Boat.ino', remote / 'Flytec_Remote.ino'])

def reachable(roots, sources):
    source_set = set(sources)
    seen, todo = set(), list(roots)
    while todo:
        path = todo.pop()
        if path in seen:
            continue
        seen.add(path)
        for name in INCLUDE.findall(path.read_text(encoding='utf-8')):
            for parent in (path.parent, ROOT / 'Boat/source', ROOT / 'Remote/source'):
                candidate = parent / name
                if candidate in source_set:
                    if candidate not in seen:
                        todo.append(candidate)
                    break
    return seen

def check_guards():
    pio = (ROOT / 'Boat/platformio.ini').read_text(encoding='utf-8')
    arduino = (ROOT / '.github/workflows/boat-target-build.yml').read_text(encoding='utf-8')
    remote = (ROOT / 'Remote/source/radio_hw_config.h').read_text(encoding='utf-8')
    sd = (ROOT / 'Remote/source/display_port.cpp').read_text(encoding='utf-8')
    required = (
        '-<heltec_sx1262_link.cpp>' not in pio,
        not (ROOT / 'Boat/source/heltec_sx1262_link.cpp').exists(),
        not (ROOT / 'Boat/source/heltec_sx1262_link.h').exists(),
        not (ROOT / 'Remote/source/platformio_entry.cpp').exists(),
        '+<*.ino>' in (ROOT / 'Remote/platformio.ini').read_text(encoding='utf-8'),
        'RadioLib @' not in pio,
        'RadioLib@' not in arduino,
        'rm /tmp/FlytecBoatBuild/heltec_sx1262_link.cpp' not in arduino,
        'rm /tmp/FlytecBoatR30H2Trace/heltec_sx1262_link.cpp' not in arduino,
        '#define REMOTE_NRF_PIN_AUDIT_SIGNED 0' in remote,
        '#define REMOTE_SD_DUMMY_CSN_PIN 6' in remote,
        '#define REMOTE_NRF_CE_PIN   (-1)' in remote,
        '#define REMOTE_NRF_CSN_PIN  (-1)' in remote,
        '#define REMOTE_NRF_MOSI_PIN (-1)' in remote,
        '#define REMOTE_NRF_SCK_PIN  (-1)' in remote,
        '#define REMOTE_NRF_MISO_PIN (-1)' in remote,
        '#define REMOTE_TF_SPI_MOSI_PIN 11' in remote,
        '#define REMOTE_TF_SPI_SCK_PIN  12' in remote,
        '#define REMOTE_TF_SPI_MISO_PIN 13' in remote,
        '#define REMOTE_R30H10_REJECTED_CE_GPIO 6' in remote,
        '#define REMOTE_R30H10_REJECTED_CSN_GPIO 15' in remote,
        '#define REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED 0' in remote,
        '#define REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED 0' in remote,
        '#error "R30H-11: no approved nRF hardware' in remote,
        'SD.begin(REMOTE_SD_DUMMY_CSN_PIN, SPI, 4000000)' in sd,
    )
    if not all(required):
        raise RuntimeError('active-source or unsigned GPIO safety invariant changed')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--json', type=pathlib.Path, required=True)
    args = parser.parse_args()
    check_guards()
    src, roots = inventory(), build_roots()
    if any(not p.exists() for p in roots):
        raise RuntimeError('missing entrypoint')
    used = reachable(roots, src)
    included_by_tests = set()
    for section in ('Boat', 'Remote'):
        for folder in ('tests', 'hardware_test'):
            for p in (ROOT / section / folder).rglob('*'):
                if p.is_file() and p.suffix in EXT:
                    included_by_tests.update(pathlib.PurePosixPath(name).name for name in
                                             INCLUDE.findall(p.read_text(encoding='utf-8')))
    hashes = defaultdict(list)
    for p in src:
        hashes[hashlib.sha256(p.read_bytes()).hexdigest()].append(rel(p))
    # Headers consumed indirectly by LVGL/ESP panel build macros are NOT dead code.
    external_config = {'Remote/source/lv_conf.h',
                       'Remote/source/esp_panel_board_custom_conf.h'}
    unused = [p for p in src if p.suffix == '.h' and p not in used
              and rel(p) not in external_config]
    report = {
        'evidence': 'STATIC_SOURCE_ONLY_NOT_PHYSICAL',
        'source_files': len(src),
        'active_compile_roots': len(roots),
        'reachable_files': len(used),
        'externally_consumed_configuration_headers': sorted(external_config),
        'test_only_headers': [rel(p) for p in unused if p.name in included_by_tests],
        'review_candidate_headers': [rel(p) for p in unused if p.name not in included_by_tests],
        'duplicate_identical_source_groups_retained': [x for x in hashes.values() if len(x)>1],
        'deprecated_lvgl_flag_calls': {
            rel(p): p.read_text(encoding='utf-8').count('lv_obj_add_flag(') +
                    p.read_text(encoding='utf-8').count('lv_obj_remove_flag(')
            for p in src if p.suffix in ('.cpp', '.ino') and
                ('lv_obj_add_flag(' in p.read_text(encoding='utf-8') or
                 'lv_obj_remove_flag(' in p.read_text(encoding='utf-8'))
        },
    }
    args.json.parent.mkdir(parents=True, exist_ok=True)
    args.json.write_text(json.dumps(report, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print('PASS: static inventory; no physical approval or release.')
    print('Source files:', len(src), 'Build roots:', len(roots),
          'review-only headers:', len(report['review_candidate_headers']))
    for candidate in report['review_candidate_headers']:
        print('REVIEW_ONLY:', candidate)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
