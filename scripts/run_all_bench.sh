#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [[ ! -x build/sort_app ]]; then
    scripts/build.sh
fi

mkdir -p Output
./build/sort_app --benchmark \
    --in dataset/input \
    --out dataset/output \
    --csv Output/benchmark.csv \
    --repeat 3 \
    --timeout 180 \
    --skip-after-tle

python3 scripts/plot_bench.py --input Output/benchmark.csv --output-dir Output
