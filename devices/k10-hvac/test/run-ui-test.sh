#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
c++ -std=c++17 -I"$ROOT/include" \
  "$ROOT/test/ui_test.cpp" \
  "$ROOT/src/ui/navigation.cpp" \
  "$ROOT/src/ui/screens.cpp" \
  -o "$ROOT/test/ui_test"
"$ROOT/test/ui_test"
