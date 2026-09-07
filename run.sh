#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Zombie Shooter - AAA Edition ==="
echo ""

if ! command -v git >/dev/null 2>&1; then
    echo "ERROR: git is not installed or not in PATH."
    exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake is not installed or not in PATH."
    echo "Please install CMake from https://cmake.org/download/"
    exit 1
fi

if [ ! -d "raylib_src" ]; then
    echo "[1/4] Cloning raylib..."
    git clone --depth 1 https://github.com/raysan5/raylib.git raylib_src
else
    echo "[1/4] raylib source found."
fi

if [ ! -f "raylib_src/build/raylib/libraylib.a" ] && [ ! -f "raylib_src/build/raylib/libraylib.lib" ]; then
    echo "[2/4] Building raylib..."
    mkdir -p raylib_src/build
    cd raylib_src/build
    cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
    make -j"$(nproc)" raylib
    cd "$SCRIPT_DIR"
else
    echo "[2/4] raylib already built."
fi

echo "[3/4] Building ZombieShooter..."
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"

cd "$SCRIPT_DIR"

echo "[4/4] Running ZombieShooter..."
if [ -f "build/ZombieShooter" ]; then
    ./build/ZombieShooter
elif [ -f "build/ZombieShooter.exe" ]; then
    ./build/ZombieShooter.exe
else
    echo "ERROR: ZombieShooter executable not found after build."
    exit 1
fi
