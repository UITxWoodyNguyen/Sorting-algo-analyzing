@echo off
setlocal
set "ROOT=%~dp0.."
pushd "%ROOT%"

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: cmake was not found in PATH.
    popd
    exit /b 1
)

set "GENERATOR="
if exist "C:\msys64\mingw64\bin\g++.exe" set "GENERATOR=-DCMAKE_CXX_COMPILER=C:\msys64\mingw64\bin\g++.exe"

cmake -S . -B build -G Ninja %GENERATOR%
if errorlevel 1 goto :fail
cmake --build build --parallel
if errorlevel 1 goto :fail

echo Build completed successfully.
popd
exit /b 0

:fail
echo Build failed.
popd
exit /b 1
