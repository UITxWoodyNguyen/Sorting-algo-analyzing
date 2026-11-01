@echo off
setlocal
set "ROOT=%~dp0.."
pushd "%ROOT%"

if not exist "build\sort_app.exe" (
    call scripts\build.bat
    if errorlevel 1 goto :fail
)

if not exist "Output" mkdir "Output"
build\sort_app.exe --benchmark --in dataset\input --out dataset\output --csv Output\benchmark.csv --repeat 3 --timeout 180 --skip-after-tle
if errorlevel 1 goto :fail

where py >nul 2>nul
if not errorlevel 1 (
    py scripts\plot_bench.py --input Output\benchmark.csv --output-dir Output
    if errorlevel 1 goto :fail
) else (
    where python >nul 2>nul
    if errorlevel 1 (
        echo ERROR: Python launcher was not found in PATH.
        goto :fail
    )
    python scripts\plot_bench.py --input Output\benchmark.csv --output-dir Output
    if errorlevel 1 goto :fail
)

popd
exit /b 0

:fail
echo Benchmark failed.
popd
exit /b 1
