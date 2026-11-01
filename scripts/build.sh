#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

command -v cmake >/dev/null || {
    echo "ERROR: cmake was not found in PATH." >&2
    exit 1
}
command -v /usr/bin/g++ >/dev/null || {
    echo "ERROR: /usr/bin/g++ was not found." >&2
    exit 1
}

cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build --parallel
echo "Build completed successfully."
