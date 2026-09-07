@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo === Zombie Shooter - AAA Edition ===
echo.

where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: git is not installed or not in PATH.
    pause
    exit /b 1
)

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: cmake is not installed or not in PATH.
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

if not exist "raylib_src\src\raylib.h" (
    echo [1/4] Cloning raylib...
    git clone --depth 1 https://github.com/raysan5/raylib.git raylib_src
    if %errorlevel% neq 0 (
        echo ERROR: Failed to clone raylib.
        pause
        exit /b 1
    )
) else (
    echo [1/4] raylib source found.
)

if not exist "raylib_src\build\raylib\raylib.lib" if not exist "raylib_src\build\raylib\libraylib.a" (
    echo [2/4] Building raylib...
    mkdir "raylib_src\build" 2>nul
    cd /d "raylib_src\build"
    cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
    if %errorlevel% neq 0 (
        echo ERROR: CMake configuration failed for raylib.
        cd /d "%SCRIPT_DIR%"
        pause
        exit /b 1
    )
    cmake --build . --config Release --target raylib
    if %errorlevel% neq 0 (
        echo ERROR: raylib build failed.
        cd /d "%SCRIPT_DIR%"
        pause
        exit /b 1
    )
    cd /d "%SCRIPT_DIR%"
) else (
    echo [2/4] raylib already built.
)

echo [3/4] Building ZombieShooter...
if exist build rmdir /s /q build
mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed for ZombieShooter.
    cd /d "%SCRIPT_DIR%"
    pause
    exit /b 1
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: ZombieShooter build failed.
    cd /d "%SCRIPT_DIR%"
    pause
    exit /b 1
)

cd /d "%SCRIPT_DIR%"

echo [4/4] Running ZombieShooter...
if exist "build\Release\ZombieShooter.exe" (
    build\Release\ZombieShooter.exe
) else if exist "build\ZombieShooter.exe" (
    build\ZombieShooter.exe
) else (
    echo ERROR: ZombieShooter.exe not found after build.
    pause
    exit /b 1
)

endlocal
