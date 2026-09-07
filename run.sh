#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RAYLIB_DIR="$SCRIPT_DIR/raylib_src"

echo "=== Zombie Shooter - AAA Edition ==="
echo ""

if [ ! -d "$RAYLIB_DIR" ]; then
    echo "[1/3] Cloning raylib..."
    git clone --depth 1 https://github.com/raysan5/raylib.git "$RAYLIB_DIR"
else
    echo "[1/3] raylib source found."
fi

if [ ! -f "$RAYLIB_DIR/build/raylib/libraylib.a" ]; then
    echo "[2/3] Building raylib..."
    mkdir -p "$RAYLIB_DIR/build"
    cd "$RAYLIB_DIR/build"
    cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc) raylib
    cd "$SCRIPT_DIR"
else
    echo "[2/3] raylib already built."
fi

echo "[3/3] Building ZombieShooter..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

echo ""
echo "Build complete! Running ZombieShooter..."
./ZombieShooter
