---
name: Raylib-GAME-DEVELOPMENT
description: "Learn raylib game programming: install raylib, understand core concepts, and build 2D/3D games with the raylib C library"
category: game-development
risk: safe
source: community
source_repo: raysan5/raylib
source_type: community
date_added: "2026-08-14"
author: community
tags: [raylib, game-development, c, graphics, 2d, 3d]
tools: [claude, cursor, gemini]
---

# Raylib-GAME-DEVELOPMENT

## Overview

This skill teaches game programming using raylib, a simple and easy-to-use C library for videogame programming. raylib is inspired by Borland BGI and XNA, and is especially well suited for prototyping, tooling, graphical applications, embedded systems, and education. This skill covers installation, core concepts, and building 2D/3D games using raylib's API and 140+ code examples.

## When to Use This Skill

- Use when you want to learn game programming with raylib
- Use when you need to create a window, handle input, draw shapes, textures, text, or 3D models
- Use when you want to build a 2D or 3D game prototype quickly
- Use when you need raylib API reference or example code patterns
- Use when working with C game development and need a simple, dependency-free library

## Requirements / Mandatory

### Tool Calls
1. Avoid any use of '&&' when running shell commands as it usually fails. Just execute commands sequentially.
2. Always no exception ask for permission to delete files or use git reset or git restore or git checkout to restore files to avoid losing untracked changes.
3. Must use sub agents for every tool call when executing skill behavior (How It Works)

### Using Agents
1. All tool calls must run using sub agents.
2. Each agent must only write to 1 file (multiple writes but only ever 1 file).
3. Agents may read from multiple files but never write to more than 1 file.
4. Must never with no exception perform any tool call like reads, edits or updates on Main agent.
5. Even small edits like 1 line of code changes must run on sub agent separate from Main Agent.
6. Only use Main Agent for communicating and orchestrating with sub agents and communicating with user.

## How It Works

### Step 1: Identify Your Goal

Before using this skill, use the `question` or `ask_user` tool to ask the user for their explicit desired goal. Grilling questions should cover:
- What type of game or application do you want to build (2D, 3D, platformer, shooter, etc.)?
- What specific raylib features do you need (window management, input, shapes, textures, models, audio, shaders)?
- What is your experience level with C and game programming?
- What platform are you targeting (Windows, Linux, macOS, Web, Android, etc.)?

Do not proceed until you have a clear, explicit goal from the user.

### Step 2: Verify Understanding

After receiving the user's goal, explicitly state back to the user:
1. In layman's terms, what you understand from their prompt
2. The goals from what you understood
3. The non-goals from what you understood
4. The plan of action
5. How the prompt could be improved (list ambiguities and how to resolve them)

Use the `question` or `ask_user` tool repeatedly until the user confirms the goals, non-goals, and optimized prompt. Only proceed when there is clear understanding of how to validate whether the goal has been achieved.

### Step 3: Verify Goal Achievement

After providing instructions, use the `question` or `ask_user` tool to ask the user how they want to verify whether the goal has been achieved. Possible verification methods include:
- The user can compile and run the provided code and confirm expected behavior
- The user can show a screenshot or recording of the running application
- The user can describe the functionality in their own words
- The user can answer specific questions about the code

Only consider the goal achieved when the user explicitly confirms success.

### Step 4: Check Session Context and Metrics

Before executing any skill behavior:
1. Check if the session context is at 50% or more. If context is at 50% or more, inform the user and offer to execute skill behavior in a new session.
2. Check for an existing `R-R-GAME-DEVELOPMENT-Execution-Metrics.md` file in the skill directory. If it exists, read and analyze it to avoid repeating previous mistakes.
3. If the metrics file does not exist, you will create it after skill execution.

### Step 5: Check Dependencies and Install if Necessary

Before creating any game or example, always verify and install required dependencies:

1. **Check for build tools in this order:**
   - CMake: Run `cmake --version`
   - GCC: Run `gcc --version`
   - If neither is found, install CMake automatically from https://github.com/Kitware/CMake/releases/

2. **Search for raylib installation across the entire system:**
   - Search all drives for `raylib.h` using system-wide search tools or grep across common directories (`C:\`, `D:\`, user directories, etc.)
   - If `raylib.h` is found, identify its parent directory and verify the complete raylib installation exists by checking for:
     - `raylib.h` in an `include` folder or the same directory
     - A corresponding library file in a `lib` folder or nearby directory:
       - `raylib.lib` or `libraylib.a` for linking
     - Optional but recommended: `src` folder with raylib source modules
   - If a complete installation is found, use that existing installation. Do not reinstall raylib.
   - If no valid installation is found, proceed to install raylib:
     - Detect the available compiler type first (MSVC or MinGW/GCC).
     - Download the matching raylib release:
       - For MSVC: `raylib-6.0_win64_msvc16.zip` from https://github.com/raysan5/raylib/releases/
       - For MinGW: `raylib-6.0_win64_mingw-w64.zip` from https://github.com/raysan5/raylib/releases/
     - Extract to `C:\raylib\` or another consistent location and use that path.
     - Verify the extracted installation contains both `include\raylib.h` and the matching library file.

3. **Match compiler and library versions:**
   - MSVC compiler requires MSVC-built raylib (`raylib.lib`)
   - MinGW/GCC compiler requires MinGW-built raylib (`libraylib.a`)
   - Never mix MinGW libraries with MSVC compiler or vice versa

4. **Reference the detected or installed raylib path in build scripts:**
   - Use the discovered or installed include and lib paths in `CMakeLists.txt` and `run.bat`
   - Do not hardcode only one absolute path unless it was actually detected/installed there

### Step 6: Plan Modular Architecture

Before writing any code, plan and create the following mandatory project structure. Do not write game logic until this structure exists:

1. **Always create a `src/` directory** in the game folder. All `.c` and `.h` module files must live inside `src/`.

2. **Always create `src/game.c` as the main entry point.** This file owns `main()`, initializes raylib, runs the game loop, and imports all other modules. No other file should contain `main()`.

3. **Always follow functional programming style with separation of concerns.** Each module must:
   - Live in its own `.c`/`.h` pair inside `src/`
   - Expose pure functions with explicit inputs and outputs
   - Avoid hidden global state; pass state through function parameters
   - Isolate side effects to explicit modules: rendering, file I/O, and input

4. **Always create these core modules** (add more only if the game requires them):
   - `src/snake.c` / `src/snake.h` — snake state and behavior
   - `src/apple.c` / `src/apple.h` — apple spawning and collision
   - `src/camera.c` / `src/camera.h` — camera follow logic
   - `src/input.c` / `src/input.h` — input handling
   - `src/renderer.c` / `src/renderer.h` — 3D scene and 2D HUD rendering
   - `src/score.c` / `src/score.h` — high score persistence

5. **Always create supporting project files**:
   - `CMakeLists.txt` that uses `file(GLOB SOURCES "src/*.c")` and builds the executable from `src/game.c`
   - `run.bat` / `run.sh` that references the `.exe` produced from the `game.c` target
   - `HOW-TO-RUN.md` documenting the project structure and how to run

6. **Only after the structure and headers are defined**, implement each module in its `.c` file, then wire them together in `src/game.c`.

### Step 7: Install raylib

Provide platform-specific installation instructions:
- **Windows**: Use the provided project templates in the `projects` directory or build via CMake/MinGW.
- **macOS**: Use Homebrew (`brew install raylib`) or build from source with CMake.
- **Linux**: Use the distribution package manager or build from source with CMake.
- **Web (HTML5)**: Use the provided Emscripten build configuration.
- **CMake (all platforms)**: Use `cmake` to configure and build raylib.

Verify installation by compiling a basic test program that includes `raylib.h` and links against `libraylib`.

### Step 8: Understand Core raylib Concepts

Explain the fundamental raylib concepts based on the user's goal:
- **Window Management**: `InitWindow()`, `CloseWindow()`, `WindowShouldClose()`, `SetTargetFPS()`
- **Drawing Loop**: `BeginDrawing()`, `EndDrawing()`, `ClearBackground()`
- **Input Handling**: Keyboard (`IsKeyPressed()`, `GetKeyPressed()`), mouse (`GetMousePosition()`, `IsMouseButtonPressed()`), gamepad, touch, and gestures
- **Shapes**: Basic shapes (circles, rectangles, triangles, polygons), lines, and collision detection
- **Textures**: Loading and drawing 2D textures with `Texture2D`
- **Text**: Font loading and text drawing with `DrawText()`
- **3D**: 3D shapes, models, cameras (`Camera3D`), billboards, and heightmaps
- **Audio**: Audio loading and playback with `Sound`, `Music`, and streaming
- **Shaders**: Custom shader loading and postprocessing
- **Math**: Vector, matrix, and quaternion operations via `raymath.h`

### Step 9: Provide Step-by-Step Instructions for the User's Goal

Based on the verified goal, provide explicit step-by-step instructions to build the desired game or application. Each step should include:
1. What code to write
2. Which raylib functions to use
3. How to compile and run the code
4. What the expected output/behavior is

Reference the examples in the `examples` directory of the cloned repository for concrete code patterns. Common example categories include:
- `core/` - Window management, input, cameras, system functionality
- `shapes/` - 2D shape drawing and collision
- `textures/` - Texture loading and manipulation
- `text/` - Font and text rendering
- `models/` - 3D model loading and rendering
- `shaders/` - Custom shader examples
- `audio/` - Sound and music examples

### Step 10: Always Create Documentation and Build Scripts

For every game or application created, always create the following files in the game directory:

1. **HOW-TO-RUN.md** - A markdown file with:
   - Platform-specific build instructions
   - Required dependencies
   - How to compile manually
   - How to run the game
   - Game controls
   - Troubleshooting section

  2. **run.bat** (Windows) or **run.sh** (Linux/macOS) - An automated build and run script that:
     - Checks for available build tools (CMake first, then GCC)
     - Installs missing build tools automatically if needed
     - Checks for raylib installation in standard locations
     - Downloads and installs raylib if missing (matching compiler version)
     - Configures the build with CMake or compiles directly with GCC
     - Builds the project
     - Runs the resulting executable produced from `src/game.c`
     - Handles errors gracefully with clear messages

3. **CMakeLists.txt** - A CMake configuration file that:
   - Uses Visual Studio 18 2026 generator on Windows
   - Finds raylib in standard locations
   - Links against required system libraries (opengl32, gdi32, winmm, shell32 on Windows)
   - Builds the game executable

### Step 11: Test Complete Build Pipeline

Before considering any game creation complete:
1. Test the complete automated build and run script end-to-end
2. Verify the game compiles without errors
3. Verify the game runs and is playable
4. Verify all features work as expected (movement, scoring, game over, etc.)
5. Document any issues found and fixes applied

### Step 12: Create Execution Metrics

After the skill execution is complete, create or append to the file `R-R-GAME-DEVELOPMENT-Execution-Metrics.md` in the skill directory. Populate it with:
- Timestamp of execution
- The user's explicit goal/prompt
- Steps taken
- Outcome (success, partial, failure)
- Any mistakes made and lessons learned
- Verification method used

This metrics file must be created every time the skill is executed, appending new entries for each session.

## Examples

### Example 1: Basic Window

```c
#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - basic window");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

### Example 2: Basic Shapes

```c
#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes");

    float rotation = 0.0f;
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        rotation += 0.2f;

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("some basic shapes available on raylib", 20, 20, 20, DARKGRAY);
            DrawCircle(screenWidth/5, 120, 35, DARKBLUE);
            DrawRectangle(screenWidth/4*2 - 60, 100, 120, 60, RED);
            DrawTriangle((Vector2){ screenWidth/4.0f *3.0f, 80.0f },
                         (Vector2){ screenWidth/4.0f *3.0f - 60.0f, 150.0f },
                         (Vector2){ screenWidth/4.0f *3.0f + 60.0f, 150.0f }, VIOLET);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Best Practices

- Always call `InitWindow()` before any other raylib function and `CloseWindow()` at the end.
- Use the game loop pattern: `while (!WindowShouldClose()) { BeginDrawing(); ... EndDrawing(); }`
- Set FPS with `SetTargetFPS()` to control frame rate.
- Use `BeginDrawing()`/`EndDrawing()` to wrap all drawing code.
- Load resources (textures, sounds, fonts) once during initialization, not inside the game loop.
- Use the examples in the `examples` directory as reference for specific features.
- **Always include all required standard library headers** (e.g., `#include <stdio.h>` for file I/O, `#include <stdlib.h>` for `rand()`/`srand()`, etc.).
- **Always verify build tool availability** before writing build scripts. Check for CMake first, then GCC.
- **Always match raylib binaries to the compiler** - MSVC needs `raylib.lib`, MinGW needs `libraylib.a`.
- **Always test the complete build pipeline** end-to-end before considering work complete.
- **Always create `HOW-TO-RUN.md` and `run.bat`/`run.sh`** for every game created.
- **Always check for existing metrics** before executing skill behavior to avoid repeating previous mistakes.
- **Always use a `src/` directory** with modular `.c`/`.h` files; never put all game code in a single root-level `.c` file.
- **Always create `src/game.c` as the entry point** containing `main()`; all other modules are imported there.
- **Always follow functional programming style**: pure functions, explicit inputs/outputs, no hidden global state, side effects isolated to rendering, file I/O, and input.
- **Always create core modules** (`snake`, `apple`, `camera`, `input`, `renderer`, `score`) as separate `.c`/`.h` pairs in `src/`.
- **Always update `run.bat`/`run.sh` and `CMakeLists.txt`** to reference the `.exe` produced from `src/game.c`, not a root-level source file.

## Limitations

- This skill does not replace environment-specific validation, testing, or expert review.
- raylib requires a C compiler and platform-specific setup; ensure your development environment is properly configured.
- Stop and ask for clarification if required inputs, permissions, or safety boundaries are missing.
- Some advanced features may require additional platform-specific dependencies or setup.

## Security & Safety Notes

- This skill includes shell commands for building raylib. Review and confirm all commands in a safe, authorized environment before execution.
- When downloading or building raylib from source, verify the source is the official `raysan5/raylib` repository.
- raylib is licensed under zlib/libpng; ensure compliance with the license when using in projects.

## Common Pitfalls

- **Problem:** raylib functions called before `InitWindow()`
  **Solution:** Always call `InitWindow()` first and ensure a valid OpenGL context exists.
- **Problem:** Drawing outside `BeginDrawing()`/`EndDrawing()` block
  **Solution:** Wrap all drawing commands between `BeginDrawing()` and `EndDrawing()`.
- **Problem:** Forgetting to link against `raylib` library
  **Solution:** Ensure your build system links against the compiled `raylib` library (e.g., `-lraylib`).
- **Problem:** Memory leaks from not unloading resources
  **Solution:** Use `UnloadTexture()`, `UnloadSound()`, `UnloadMusicStream()`, and `UnloadFont()` to free resources.
- **Problem:** Missing standard library includes (e.g., `#include <stdio.h>` for file I/O)
  **Solution:** Always explicitly include all required standard headers; do not rely on transitive includes from other headers.
- **Problem:** Mismatched compiler and library binaries (MinGW library with MSVC compiler)
  **Solution:** Download raylib binaries that match your compiler: `raylib-6.0_win64_msvc16.zip` for MSVC, `raylib-6.0_win64_mingw-w64.zip` for MinGW.
- **Problem:** Assuming build tools are available without verification
  **Solution:** Always check for CMake/GCC with `cmake --version` and `gcc --version` before writing build scripts.
- **Problem:** CMake generator mismatch (e.g., Ninja without Ninja installed)
  **Solution:** Use the simplest available generator: prefer Visual Studio 18 2026 on Windows with MSVC, or Ninja if explicitly installed.
- **Problem:** Not testing the complete build pipeline end-to-end
  **Solution:** Always run the full `run.bat` or manual build commands to verify compilation, linking, and execution before considering the task complete.
- **Problem:** Putting all game code in a single root-level `.c` file instead of `src/game.c`
  **Solution:** Always create a `src/` directory with modular `.c`/`.h` files; `src/game.c` must be the only entry point with `main()`.
- **Problem:** Mixing 2D HUD/draw-text calls inside `BeginMode3D()`/`EndMode3D()`
  **Solution:** Draw 3D objects inside 3D mode only; draw HUD and overlays in 2D mode after `EndMode3D()`.
- **Problem:** Using global variables instead of passing state through functions
  **Solution:** Follow functional programming style: pass state as function parameters and return new state; avoid hidden global state.
- **Problem:** Header forward declaration errors from circular dependencies
  **Solution:** Map the full include graph before writing any header. If two modules need each other's types, extract shared types into a separate common header or use forward declarations consistently.
- **Problem:** Missing includes in implementation files
  **Solution:** Every .c file must explicitly include every header that declares functions/types it uses. Do not rely on transitive includes.
- **Problem:** Struct member name/type mismatches between header and implementation
  **Solution:** The header is the single source of truth. After writing a .c file, grep for `struct->member` and verify every member exists in the header with the correct type.
- **Problem:** Using non-existent raylib API functions or wrong signatures
  **Solution:** Always verify raylib function names and signatures against the installed `raylib.h` or official examples. Never assume API names.
- **Problem:** Duplicate definitions from incremental sed edits
  **Solution:** Avoid sed for complex file edits. Use the edit tool with exact oldString/newString pairs, or rewrite the entire file cleanly.
- **Problem:** Stale CMake cache causing source-path mismatch errors
  **Solution:** Always delete the build/ directory before running CMake in automation scripts.
- **Problem:** Hardcoded absolute paths in scripts or CMake
  **Solution:** Use only relative paths in all scripts (e.g., `raylib_src`, `src`, `build`). CMake may use `CMAKE_CURRENT_SOURCE_DIR` internally, but scripts must not hardcode absolute paths.
- **Problem:** Windows library detection failure (LNK1181)
  **Solution:** Check for raylib libraries in all plausible locations: `raylib_src/build/raylib/` and `raylib_src/build/raylib/Release/`. Support both `raylib.lib` (MSVC) and `libraylib.a` (MinGW).
- **Problem:** CMake cannot find raylib headers or libraries
  **Solution:** Use `find_path()` and `find_library()` in CMakeLists.txt with `NO_DEFAULT_PATH` to search only the project's `raylib_src` directory. Provide clear error messages if not found.

## Related Skills

- `@related-skill` - Related Skill

## Judge Protocol

This protocol applies whenever visual fidelity is being evaluated against concept art or reference imagery. It enforces a self-assessment loop before external review, using tiered judging criteria with gated scoring and clear exit conditions.

### Self-Assessment Loop

Before submitting any work for review:
1. Review the candidate output yourself
2. Ensure it actually achieves the stated goals
3. Do not submit half-baked work
4. Step back and look at the output and concept side-by-side
5. Log an honest assessment of whether it is judge-ready
6. Only submit if you are confident you have significantly improved the score
7. Be rigorous, objective, and transparent
8. Look at every pixel and detail
9. Check for missing/incorrect objects, wrong scale, perspective, positioning
10. Check for rendering glitches, flat untextured surfaces, ugly lighting, poor contrast, speckles, ugly shadows
11. Scan surface by surface, object by object, audit everything

### Judge Prompt

When submitting a screenshot to the judge, use this prompt with the latest screenshot, concept image, and previous verdict if available:

```
You are an art director reviewing a real-time render against its concept art. Compare the screenshot to the concept and score it 0-10 using this ladder. The ladder is gated: a frame cannot score above a tier's cap until every requirement of the tiers below it is fully met. Be strict about the gates.

Tier 1, shape (0-3): camera, framing, composition, and the position and rough scale of every major object match the concept. This is about layout, not finish or precision: every major element is present, in the right region of the frame (within about 10% of frame width/height), at roughly the right size (within about 25%). An object the right place and vaguely correct outline passes, even if its edges and surface are wrong. Don't be nitpicky about precision, save that for Tier 4. The goal is just to have the right elements present in roughly the right spot at this tier. Cap 3 until this is true.
Tier 2, light and color (3-5): key light direction and color, overall exposure (no clipping to black or white), shadow depth, palette, contrast, and atmosphere. Pay attention to reflections, glows, etc, and ensure they look great. Ensure the scene overall is not too bright or too dark relative to the concept. Judge at the level of the whole frame, not individual tiny details; those are Tier 4 polish. Cap 5 until the overall lighting, reflections, color, and contrast is generally right.
Tier 3, materials and surfaces (5-7): every surface reads as the right material at a glance: Textures, roughness, translucency, wetness, reflections. Ensure assets don't look obviously procedural, blocky, simple, smooth/plastic; push for elements that dominate the frame to be properly sculpted and detailed (Blender assets with high quality image-gen textures). Cap 7 until this is true.
Tier 4, fine detail (7-9): the small things: texture and fine detail. Nitpick relentlessly. Look at every little object up close. Layout should align near-perfectly with the concept. Materials should look extremely convincing. Cap 9 until they are right.
Tier 5, indistinguishable (9-10): holds up side by side and zoomed in. Nitpick every pixel.

If a previous verdict and screenshot are provided: you are one reviewer in a sequence, not the first. Maintain consistency. First go through the previous directives one by one and mark each LANDED, PARTIAL, or NOT DONE based on the new screenshot. Carry forward anything PARTIAL or NOT DONE. Do not reverse a prior directive unless the result is clearly worse than before, and if you do, say so explicitly and why.

Output format:

The score on the first line, then "Tier N" on the second line: the highest tier whose gate is fully passed. 1b. If given a previous verdict: the LANDED / PARTIAL / NOT DONE list for its directives.
"Blocking:" the specific things that fail the gate of the next tier. These come first and the builder must clear them before anything else counts. Name the element and say what to change, with magnitudes: "Rocks: replace the stacked ovoid boulders with one continuous fractured slab; cracks 2-5cm wide, dark interiors, add more texture to the surfaces so they don't look flat/plastic" not just "the rocks look artificial".
Then at most 4 further directives from higher tiers, same style, ordered by points recoverable.
Don't give non-actionable feedback like "This element looks synthetic." Name the specific things causing that impression. Every directive must be something a developer can act on this round. Don't round up score: if a gate is not fully passed, the cap holds.
```

### Exit Criteria

- **Score >= 8 and target FPS acceptable**: Done! Show the user the latest screenshot and ask if they want more iterations.
- **Score >= 8 but target FPS unacceptable**: Optimize, aiming for lossless wins first, then optimizations with minimal visual impact. Re-judge after optimizations.
- **Stall approaching**: Best score hasn't improved by a full point in 2 rounds, or judge named same gap 3 times. Stop incremental tweaks. Make a big structural change: swap asset strategy, rewrite lighting model, rebuild composition, change camera. Self-check before judge.
- **Stalled**: At least one big structural change was tried and score still hasn't improved in 3 rounds, or judge asks for intractable improvements. Stop and tell the user why blocked, give options.
- **None of the above**: Address most heavy-hitting gaps this round. Prioritize highest-impact changes. Only revert if score dropped by a full point or more.

### Visual Quality Requirements

Do not settle for plain, flat, procedural looks for key environmental details. Use image generation tools for textures, normal maps, skyboxes, etc. when available. The tiny details and texturing matter and require custom sculpting or high-quality assets.

### Target Resolution

Target the same resolution and aspect ratio as the concept art for fair comparison.

### Subagent Judge

Judging should ideally be done by a fresh subagent with a clean context. Give the judge the latest screenshot, concept image, and previous verdict/screenshot if available.
