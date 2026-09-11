# Agent Instructions

## Rendering Rules

### 1. Render Texture Lifecycle

Every render texture target MUST follow this exact lifecycle:

```c
// BEGIN
BeginTextureMode(target);
ClearBackground(clearColor);
BeginMode3D(camera);
// ... draw 3D scene ...
EndMode3D();
EndTextureMode();
// BLIT to backbuffer BEFORE any 2D overlays
DrawTextureRec(target.texture, (Rectangle){0, 0, width, -height}, (Vector2){0, 0}, WHITE);
// NOW draw 2D HUD, text, overlays on the backbuffer
```

**Rules:**
- `BeginTextureMode()` MUST be paired with `EndTextureMode()` in the same function
- After `EndTextureMode()`, the texture MUST be drawn to the backbuffer before HUD
- Never draw HUD or billboards while still inside texture mode
- If you add `BeginTextureMode()`, you must add the matching blit immediately

### 2. PBR Shader Output Contract

The PBR fragment shader MUST output LDR color in [0,1] range. The required pipeline is:

```
albedo + lighting (HDR) → tone map → gamma correct → finalColor
```

**Required stages (in order):**
1. `color = color / (color + vec3(1.0))` — Reinhard tonemapping
2. `color = pow(color, vec3(1.0 / 2.2))` — Gamma correction

**Required BRDF components:**
- `DistributionGGX` for specular distribution
- `GeometrySmith` for geometry attenuation
- `FresnelSchlick` for Fresnel term
- Full light loop: `(kD * albedo / PI + specular) * radiance * NdotL`
- Directional light integration with same BRDF

**Ambient constraints:**
- Base ambient MUST be `vec3(0.04-0.06) * albedo * ao`
- NEVER use `albedo * 0.55` or any ambient > 0.08 without indirect lighting

**If removing complexity:**
- Keep full GGX BRDF if normal maps or metallic/roughness are used
- At minimum, keep tone map + gamma if reducing lighting model

### 3. Post-Processing Shader

The post-process shader MUST apply at least gamma correction if the PBR shader does not.

Current post-process output: `pow(col, vec3(1.0 / 2.2))` — keep this.

If you add ACESFilm or other tone mapping to post-process, ensure it's not duplicated in PBR.

### 4. Model Replacement Rules

When replacing any mesh/model:

1. Measure the original bounding box against constants in the header (`TORSO_WIDTH`, `TORSO_HEIGHT`, `HEAD_RADIUS`, `LIMB_RADIUS`, `ARM_*_LEN`, `LEG_*_LEN`)
2. New mesh MUST have equal or greater visual mass
3. Y-position math must be recalculated from actual bone lengths, not hardcoded offsets
4. Original silhouette must not shrink without explicit user approval

### 5. Shader Uniform Updates

When setting shader uniforms in `RendererDrawZombies()` or similar:

- Light colors should use `lightCol[i] * attenuation` (no extra `* 1.2` multiplier)
- Light attenuation should use proper distance falloff: `1.0 / (1.0 + 0.05 * dist + 0.01 * dist * dist)`
- Fire light flicker is ±15-25% — without tonemapping this will cause visible white flash

### 6. Pre-Commit Rendering Checklist

Before committing any rendering change:
- [ ] `BeginTextureMode` has matching `EndTextureMode` + blit
- [ ] PBR shader has tone map AND gamma correction
- [ ] PBR shader has full GGX BRDF if using normal/metallic
- [ ] Ambient is ≤ 0.08 * albedo * ao
- [ ] Model replacements match or exceed original scale
- [ ] Build succeeds with zero errors
- [ ] Screenshot taken and visually verified (no blown whites, scene visible)

### 7. Failure Symptoms and Causes

| Symptom | Cause | Fix |
|---------|-------|-----|
| Screen shows only flat color | `EndTextureMode` missing or blit missing | Add `EndTextureMode()` + `DrawTextureRec` |
| Scene visible but blown-out white | Missing tonemap or gamma in PBR | Add Reinhard + gamma to PBR shader |
| Visible white flash pulsing | Fire lights + unconstrained HDR | Add tonemap, or reduce light intensity |
| Zombies invisible | Model scale too small | Compare against original constants |
| HUD invisible | Drawn inside texture mode | Draw HUD after blit, on backbuffer |
| Zombie heads at wrong positions | `GetWorldToScreen` called inside texture mode | Call after `EndTextureMode`, on backbuffer |
| Player/gun invisible on first load | `PlayerInit`/`WeaponInit` called before `RendererInit` | Call `RendererInit` first so shader is valid |
| Crouch snaps or doesn't work | Target/current value confused in lerp | Use separate `crouchTarget` and `crouchAmount` |

## Game Logic Rules

### 8. Initialization Order

`GameInit()` MUST follow this order:
1. `RendererInit()` — initializes shaders first
2. `PlayerInit()` — needs valid shader for model materials
3. `WeaponInit()` — needs valid shader for model materials
4. `CameraInit()` — doesn't need shader
5. `AudioInit()`, `TextureGenerate()`, etc.

### 9. Main Loop State Machine

`main()` MUST:
- Start with `menu.active = true` and `state = GAME_STATE_MENU`
- Only call `GameInit()` from `UIUpdate()` when user starts game, or from auto-start path
- Never call `GameInit()` directly in `main()` before the main loop

### 10. Input Handling Rules

| Action | Input | Implementation |
|--------|-------|----------------|
| Shoot | Left click press | `mouseLeftPressed` (edge trigger) |
| Hold for 1st person | Right click hold | `mouseRightDown` (level trigger) |
| Toggle camera mode | C key | `cameraTogglePressed` |
| Sprint | Shift hold | `shiftPressed` |
| Crouch | Ctrl hold | `ctrlPressed` |
| Jump | Space press | `spacePressed` |

### 11. Camera Mode System

- `baseMode` — persistent mode set by C key toggle (3rd/1st person)
- `mode` — current mode, can be temporarily overridden by right-click hold
- Right-click hold forces `mode = FIRST_PERSON`, release restores `mode = baseMode`
- `CameraUpdate()` lerps `firstPersonBlend` based on `mode`

### 12. Crouch Implementation

- `CameraSetCrouch()` sets `crouchTarget` to 1.0 (crouching) or 0.0 (standing)
- `CameraUpdate()` lerps `crouchAmount` toward `crouchTarget` at 8.0 units/sec
- Crouch offset: `crouchAmount * 0.7f` subtracted from both 3rd-person and 1st-person camera Y

### 13. Jump Physics

- `Player` struct has `velocityY` and `isGrounded`
- Jump: `velocityY = PLAYER_JUMP_FORCE` on space press when grounded
- Gravity: `velocityY -= PLAYER_GRAVITY * dt` each frame
- Ground check: if `position.y <= 0`, set `velocityY = 0` and `isGrounded = true`

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

## Screenshot Workflow

### Directory and Naming Convention

All workflow screenshots are saved to the `workflow/` directory using the `iteration_XX.png` naming convention, where `XX` is an incrementing zero-padded count starting from `01`.

Examples:
- `workflow/iteration_01.png`
- `workflow/iteration_02.png`
- `workflow/iteration_10.png`

### Environment Variables

| Variable | Purpose |
|----------|---------|
| `ZOMBIE_AUTO_START=1` | Skips menu and starts gameplay automatically after 30 frames |
| `ZOMBIE_AUTO_QUIT_MS=5000` | Auto-quits after the specified milliseconds (prevents hangs) |
| `ZOMBIE_SCREENSHOT_ROTATE=1` | Auto-rotates camera 360° and captures screenshots at the interval set by `ZOMBIE_SCREENSHOT_STEP_DEGREES` |
| `ZOMBIE_SCREENSHOT_STEP_DEGREES=45` | Rotation step in degrees; total screenshots = 360 / step (default 90° = 4 screenshots) |
| `ZOMBIE_SCREENSHOT_WORLD=1` | Spawns world only (no zombies) and captures rotating screenshots |
| `ZOMBIE_SCREENSHOT_ZOMBIE=1` | Spawns 1 zombie only (blank background) and captures rotating screenshots |
| `ZOMBIE_SCREENSHOT_PLAYER=1` | Spawns player only (blank background) and captures rotating screenshots |

### Command Template

```bash
cd /workspace/.../sessions/agent_xxx/build
ZOMBIE_AUTO_START=1 ZOMBIE_AUTO_QUIT_MS=5000 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./ZombieShooter
```

### Screenshot Modes

Use one of these environment variables to control what is rendered:

```bash
# Full world, no zombies
ZOMBIE_SCREENSHOT_WORLD=1

# Single zombie on blank background
ZOMBIE_SCREENSHOT_ZOMBIE=1

# Player only on blank background
ZOMBIE_SCREENSHOT_PLAYER=1
```

Combine with `ZOMBIE_SCREENSHOT_ROTATE=1` to capture 4 angles at 90° intervals:

```bash
ZOMBIE_AUTO_START=1 ZOMBIE_SCREENSHOT_ROTATE=1 ZOMBIE_SCREENSHOT_ZOMBIE=1 ZOMBIE_AUTO_QUIT_MS=30000 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./ZombieShooter
```

### Step-by-Step Workflow

1. **Build first**: `cd build && make -j"$(nproc)"`
2. **Run headless** with xvfb using the template above
3. **Verify screenshot exists**: `ls -la workflow/iteration_*.png`
4. **Analyze screenshot** with Python/PIL:
   ```python
   from PIL import Image
   img = Image.open('workflow/iteration_XX.png')
   print(f'Size: {img.size}, Mode: {img.mode}')
   pixels = list(img.getdata())
   non_transparent = sum(1 for p in pixels if len(p) < 4 or p[3] > 128)
   brightness = sum(sum(p[:3])/3 for p in pixels if len(p) >= 3) / len(pixels)
   white_count = sum(1 for p in pixels if len(p) >= 3 and p[0] > 240 and p[1] > 240 and p[2] > 240)
   print(f'Non-transparent: {non_transparent}/{len(pixels)}')
   print(f'Average brightness: {brightness:.1f}')
   print(f'White-ish pixels: {white_count}')
   ```
5. **Visual inspection**: print the screenshot or inspect pixel statistics to confirm the scene is visible, not washed out, and contains expected elements

### Screenshot Timing

The screenshot is taken at **frame 35** by default (`screenshotFrame = 35` in `src/game.c`). Auto-start triggers at **frame 30** (`autoStartFrame = 30`). This gives 5 frames of gameplay before capture.

### Common Failure Patterns

| Screenshot Result | Likely Cause |
|-------------------|--------------|
| File not created | Game crashed before frame 35; check stderr |
| Solid flat color | `EndTextureMode` missing; scene rendered off-screen |
| Mostly white / blown out | PBR shader missing tonemap or gamma |
| Pulsing bright flash | Fire lights with unconstrained HDR |
| Tiny/empty scene | Model scale too small or camera at wrong position |
| HUD missing | HUD drawn inside texture mode, not on backbuffer |
| Player/gun invisible on first load | `PlayerInit`/`WeaponInit` called before `RendererInit` |

### Systematic Pixel Analysis Algorithm

When analyzing screenshots, follow this **broad-to-narrow** iterative approach:

#### Step 1: Analyze ALL pixels first
```python
from PIL import Image
img = Image.open('workflow/iteration_XX.png')
width, height = img.size
pixels = list(img.getdata())
print(f'Total pixels: {len(pixels)}')
print(f'Size: {width}x{height}')
```

#### Step 2: Get global statistics
```python
brightness = sum(sum(p[:3])/3 for p in pixels if len(p) >= 3) / len(pixels)
white_count = sum(1 for p in pixels if len(p) >= 3 and p[0] > 240 and p[1] > 240 and p[2] > 240)
print(f'Average brightness: {brightness:.1f}')
print(f'White-ish pixels: {white_count}')
```

#### Step 3: Identify dominant colors across entire image
```python
from collections import Counter
color_counts = Counter()
for p in pixels:
    r, g, b = p[:3]
    qr, qg, qb = r // 16, g // 16, b // 16
    color_counts[(qr, qg, qb)] += 1

top_colors = color_counts.most_common(10)
for color, count in top_colors:
    r, g, b = color[0] * 16 + 8, color[1] * 16 + 8, color[2] * 16 + 8
    print(f'RGB({r:3d},{g:3d},{b:3d}): {count:6d} pixels ({count/len(pixels)*100:.1f}%)')
```

#### Step 4: Filter by target color across ENTIRE image
```python
# Example: find all blue pixels
blue_pixels = []
for y in range(height):
    for x in range(width):
        p = pixels[y * width + x]
        r, g, b = p[:3]
        if b > 80 and b > r + 5 and b > g + 5:
            blue_pixels.append((x, y, p[:3]))

print(f'Blue pixels: {len(blue_pixels)}')
```

#### Step 5: Cluster and analyze filtered pixels
```python
from collections import Counter
y_counts = Counter(y for x, y, c in blue_pixels)
top_y = y_counts.most_common(10)
print('Top y positions:')
for y, count in top_y:
    xs = [x for x, yy, c in blue_pixels if yy == y]
    print(f'  y={y}: {count} pixels, x range {min(xs)}-{max(xs)}')
```

#### Step 6: Refine search based on clusters
- If one cluster is dominant (e.g., sky), filter it out
- Focus on remaining clusters
- Sample pixels around suspected target areas

**Key Principle:** Always analyze the **entire image first**, then iteratively filter. Never start with a small region unless you already know the target location from prior analysis.

### Rules

- Always take a screenshot after rendering changes before declaring success
- If brightness > 200 average, the scene is likely blown out — check PBR output stages
- If non-transparent pixels < 50% of frame, the scene may not be rendering to the backbuffer
- Use `xvfb-run -a` (auto-select display) to avoid conflicts with existing X servers
- Use `-screen 0 1280x720x24` to match the game's window size and color depth
