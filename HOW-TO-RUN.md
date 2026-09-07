# Zombie Shooter - AAA Edition

## Project Structure

```
zombie-game/
├── src/
│   ├── game.c              - Main entry point, game loop, state machine
│   ├── game.h              - Game state struct and public API
│   ├── zombie.c / .h       - Zombie AI, spawning, rendering
│   ├── player.c / .h       - Player movement, health, collision
│   ├── weapon.c / .h       - Gun mechanics, shooting, scope
│   ├── camera.c / .h       - Third-person camera follow
│   ├── input.c / .h        - Keyboard/mouse input handling
│   ├── renderer.c / .h     - 3D scene and 2D HUD rendering
│   ├── ui.c / .h           - Startup menu, mode selection, image upload
│   ├── audio.c / .h        - Procedural audio generation
│   ├── particle.c / .h     - Particle effects (blood, muzzle flash)
│   ├── shader.c / .h       - PBR and post-processing shaders
│   ├── texture.c / .h      - Procedural texture generation
│   └── image_upload.c / .h - Image loading and drag-and-drop
├── raylib_src/             - raylib source (cloned from GitHub)
├── CMakeLists.txt          - CMake build configuration
├── run.sh                  - Build and run script (Linux/macOS)
├── run.bat                 - Build and run script (Windows)
└── HOW-TO-RUN.md           - This file
```

## Dependencies

- GCC 9+ or Clang
- CMake 3.16+
- X11 development libraries (Linux)
- OpenGL development libraries

## Building

### Linux / macOS

```bash
chmod +x run.sh
./run.sh
```

Or manually:
```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./ZombieShooter
```

### Windows (MinGW)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j%NUMBER_OF_PROCESSORS%
ZombieShooter.exe
```

### Windows (MSVC)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026"
cmake --build . --config Release
Release\ZombieShooter.exe
```

## Web / HTML5 (Emscripten)

To build for Web/HTML5, you need Emscripten SDK installed:

```bash
emcmake cmake .. -B build_web
cmake --build build_web
emrun build_web/ZombieShooter.html
```

## Controls

- **WASD** - Move player
- **Mouse** - Look around
- **Left Click** - Shoot
- **Right Click** - Toggle sniper scope
- **R** - Reload
- **ESC** - Pause / Return to menu

## Game Modes

### Start Menu
1. **Mode**: Choose between Rounds (wave-based) or Endless
2. **Zombie Mode**: Choose between Mixed (mostly default zombies, occasional image-head) or All Images (every zombie uses an uploaded image as head)
3. **Upload Images**: Drag and drop PNG/JPG images onto the window when prompted, or use the file picker

### Gameplay
- Zombies spawn in waves
- Defeat all zombies to advance to the next wave
- Survive as long as you can in Endless mode
- Uploaded images are randomly assigned as zombie heads
- Use the sniper scope (right click) for precision shots

## Troubleshooting

### "GLFW: Failed to initialize GLFW"
- Ensure you have a display server running (X11 on Linux, etc.)
- On headless servers, use a virtual framebuffer: `Xvfb :99 -screen 0 1024x768x24 & export DISPLAY=:99`

### "No rule to make target '../raylib_src/build/libraylib.a'"
- Run `./run.sh` which will automatically clone and build raylib
- Or manually build raylib: `cd raylib_src && mkdir build && cd build && cmake .. && make -j$(nproc) raylib`

### "Segmentation fault"
- Ensure your graphics drivers are up to date
- Try running with software rendering: `LIBGL_ALWAYS_SOFTWARE=1 ./build/ZombieShooter`

## AAA Features Implemented

- **PBR-like shading**: Custom GLSL shaders with metallic/roughness workflow
- **Dynamic lighting**: Multiple point lights with attenuation
- **Particle effects**: Blood splatter, muzzle flash, debris
- **Post-processing**: Vignette, film grain, bloom
- **Sniper scope**: Circular scope overlay with crosshair
- **Procedural audio**: Synthesized gunshots and zombie sounds
- **Procedural textures**: All zombie skins, decals, and environments generated at runtime
- **Image upload**: Drag-and-drop or file picker for custom zombie head textures

## Known Limitations

- Image upload requires drag-and-drop or file picker (HTML5 version needs Emscripten)
- Zombie models are basic procedural geometries (cylinders/spheres)
- No skeletal animation (procedural bobbing only)
- Limited zombie AI (direct pursuit)
- Single weapon type

## License

raylib is licensed under zlib/libpng. This project follows the same license.
