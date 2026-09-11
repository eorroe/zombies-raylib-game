# Agent Instructions

## Crash Prevention Rules

### Windows Compatibility

- Never pass `NULL` as the second argument to `_findfirst()` on Windows; always declare a `struct _finddata_t` variable and pass its address.
- Wrap POSIX-only headers like `dirent.h` in `#if defined(_WIN32) || defined(_WIN64)` and provide a Windows implementation using `_findfirst/_findnext`.
- Do not assume `dirent.h`, `unistd.h`, or other POSIX headers exist on Windows.
- Always test cross-platform code paths on both Linux and Windows when possible.

### Initialization Order

- `GameInit()` MUST follow this order:
  1. `RendererInit()` — initializes shaders first
  2. `PlayerInit()` — needs valid shader for model materials
  3. `WeaponInit()` — needs valid shader for model materials
  4. `CameraInit()` — doesn't need shader
  5. `AudioInit()`, `TextureGenerate()`, etc.

### Memory and Resource Management

- Always check return values from resource-loading functions before using handles.
- Never use uninitialized pointers or textures; validate IDs before drawing.
- Shutdown order should roughly reverse initialization order.

### Main Loop Stability

- Never call `GameInit()` directly in `main()` before the main loop.
- Only call `GameInit()` from `UIUpdate()` when the user starts the game, or from the auto-start path.
- Ensure `autoStartFrame` is set to a safe value that gives resources time to load.
- The auto-quit timer must be checked before taking screenshots or performing expensive operations.
