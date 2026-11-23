#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
g++ -std=c++17 -O1 -pthread -Wall -Wextra -Wpedantic -Werror \
 -I"$ROOT/Boat/tests/r30h73_freertos_mock" \
 -I"$ROOT/Boat/source" \
 "$ROOT/Boat/tests/r30h73_transport_queue_host.cpp" -o "$TMP/r30h73"
"$TMP/r30h73"
