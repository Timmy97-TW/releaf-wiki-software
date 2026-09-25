#!/bin/sh
# Protocol round-trip test: compile the sketch for the desktop, run six
# scenarios, then feed the lines it emits into the operator page's own parser.
# No board and no browser needed.
#
#   sh "Digital Twin/06_Firmware/test/run.sh"
set -e
cd "$(dirname "$0")/.."
OUT="${TMPDIR:-/tmp}/releaf_fw_test"
g++ -std=c++17 -I. -o "$OUT" test/harness.cpp
"$OUT" > "$OUT.txt"
echo "--- firmware output ---"
cat "$OUT.txt"
echo "--- page parser ---"
node test/protocol_test.js "$OUT.txt"
