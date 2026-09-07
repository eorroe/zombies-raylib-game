@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set RAYLIB_DIR=%SCRIPT_DIR%raylib_src

echo === Zombie Shooter - AAA Edition ===
echo.

if not exist "%RAYLIB_DIR%" (
    echo [1/3] Cloning raylib...
    git clone --depth 1 https://github.com/raysan5/raylib.git "%RAYLIB_DIR%"
) else (
    echo [1/3] raylib source found.
)

if not exist "%RAYLIB_DIR%\build\raylib\raylib.lib" (
    echo [2/3] Building raylib...
    mkdir "%RAYLIB_DIR%\build"
    cd /d "%RAYLIB_DIR%\build"
    cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release --target raylib
    cd /d "%SCRIPT_DIR%"
) else (
    echo [2/3] raylib already built.
)

echo [3/3] Building ZombieShooter...
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

echo.
echo Build complete! Running ZombieShooter...
Release\ZombieShooter.exe
