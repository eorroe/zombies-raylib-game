# Agent Instructions

## Mandatory Visual Update Protocol

**Before making ANY update—code, rendering, or otherwise—you MUST:**

1. **STOP** and read the **Screenshot Workflow** section in full
2. **STOP** and read the **Judge Protocol** section in full
3. **BUILD** the project and verify it compiles with zero errors
4. **RUN** the built executable in headless mode (`xvfb-run -a -s "-screen 0 1280x720x24"`) and verify it actually executes without crashing
5. Execute the screenshot workflow after your change
6. Apply the Judge Protocol to verify the result
7. Do NOT declare the update complete until the Judge Protocol confirms success
8. Do NOT commit, push, or report to the user until steps 3–7 are verified

This applies to **every** update, including:
- Visual/rendering changes
- Shader changes
- Color changes
- Code changes that might affect output
- "Small" or "trivial" changes

**If you have not executed the screenshot workflow and applied the Judge Protocol, the update is NOT complete.**

## Rendering Rules

### 1. Branch Policy

**ONLY work on the `doodle-style` branch in this session.**

- All commits MUST be made to `doodle-style`
- NEVER create, push, or update any other branch
- NEVER push to `main`, `master`, or any branch other than `doodle-style`
- If you find yourself on another branch, switch to `doodle-style` immediately
- Before committing, verify: `git branch --show-current` must output `doodle-style`

### 2. Render Texture Lifecycle

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

### 5.1 Mandatory Screenshot Workflow for Every Visual Update

**For EVERY visual change—without exception—you MUST complete the full screenshot workflow before considering the change finished.**

This is not optional. A visual change is NOT complete until the screenshot workflow has been executed and the result has been judged against the reference.

**Required steps for every visual update:**
1. Make the code change
2. Build the project
3. Run headless with `xvfb-run -a -s "-screen 0 1280x720x24"` using the `workflow/` screenshot environment variables
4. Capture a screenshot to `workflow/iteration_XX.png`
5. **Apply the Judge Protocol** (systematic pixel analysis) to compare against the reference image
6. Only declare success if the Judge Protocol confirms the visual change matches intent
7. If the result does not match, iterate: adjust the code, rebuild, recapture, re-judge

**What qualifies as a visual update:**
- Shader changes (post-process, PBR, material)
- Color palette changes
- Model/material/tint changes
- Render pipeline changes (render textures, blits, post-processing order)
- Lighting changes
- HUD/overlay changes
- Any change that affects what appears on screen

**Do NOT skip the screenshot workflow for "small" or "obvious" visual changes.**

### 6. Pre-Commit Rendering Checklist

Before committing any rendering change:
- [ ] `BeginTextureMode` has matching `EndTextureMode` + blit
- [ ] PBR shader has tone map AND gamma correction
- [ ] PBR shader has full GGX BRDF if using normal/metallic
- [ ] Ambient is ≤ 0.08 * albedo * ao
- [ ] Model replacements match or exceed original scale
- [ ] Build succeeds with zero errors
- [ ] Screenshot taken and visually verified using the Judge Protocol (Section 5.1/5.2)

### 7. Doodle Style Rendering Rules

The current target style is **blue pen ink on notebook paper**. All rendering changes must respect this style.

#### 7.1 Color Palette

| Element | Color | Purpose |
|---------|-------|---------|
| Paper background | `(245, 240, 232)` | Cream notebook paper |
| Ink blue (primary) | `(0.08, 0.18, 0.52)` | Main ink color |
| Ink cyan (secondary) | `(0.06, 0.36, 0.60)` | Variation in ink tone |
| Dark ink | `(20, 30, 60)` | Outlines, wireframes, text |
| Medium blue ink | `(40, 80, 160)` | HUD elements, crosshair |
| Light blue ink | `(100, 140, 180)` | Ground, distant objects |
| Red ink (warning) | `(120, 40, 40)` | Damage, reloading alerts only |

#### 7.2 Post-Processing Pipeline

The post-process shader MUST be applied every frame. The required pipeline is:

```c
// In RendererEnd:
EndMode3D();
EndTextureMode();

// Blit scene to backbuffer
DrawTextureRec(sceneTarget.texture, ...);

// Apply post-process shader
BeginTextureMode(postProcessTarget);
ClearBackground(BLANK);
BeginShaderMode(game->shaders.postProcess);
DrawTextureRec(sceneTarget.texture, ...);
EndShaderMode();
EndTextureMode();

// Blit final result to backbuffer
DrawTextureRec(postProcessTarget.texture, ...);

// NOW draw HUD on backbuffer
```

#### 7.3 Post-Process Shader Requirements

The post-process shader MUST include:
- Paper background with notebook ruled lines (horizontal + vertical margin)
- Blue ink edge detection
- Color quantization for hand-drawn banding effect
- Paper grain noise
- Gamma correction at the end
- NO additional gamma if PBR already applies it

#### 7.4 Scene Color Guidelines

- All hardcoded scene colors MUST use the blue ink palette
- No warm earth tones (browns, oranges, reds) except for damage/warnings
- Wireframes and outlines MUST be dark ink `(20, 30, 60)`
- Blood decals MUST be dark blue ink, not red
- Particle colors MUST be blue ink tones

#### 7.5 Model Color Guidelines

- All model tint colors MUST be blue ink tones
- Player, zombie, and weapon models MUST NOT use white or warm skin tones
- Use `(60, 100, 160)` range for primary model ink color

#### 7.6 HUD Guidelines

- HUD background MUST be light paper `(230, 235, 240, 200)`
- HUD text and borders MUST be dark ink `(20, 30, 60)`
- Health bar MUST be medium blue ink
- Crosshair MUST be medium blue ink
- Warnings (reloading, low health) MAY use red ink

### 8. Failure Symptoms and Causes

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
| Scene not blue ink style | Post-process shader not applied or colors wrong | Apply post-process every frame, use blue ink palette |
| Paper lines missing | Post-process shader missing notebook line code | Add horizontal ruled lines and vertical margin |
| Double gamma / washed out | Gamma applied in both PBR and post-process | Keep gamma in only one shader, not both |

## Game Logic Rules

### 9. Initialization Order

`GameInit()` MUST follow this order:
1. `RendererInit()` — initializes shaders first
2. `PlayerInit()` — needs valid shader for model materials
3. `WeaponInit()` — needs valid shader for model materials
4. `CameraInit()` — doesn't need shader
5. `AudioInit()`, `TextureGenerate()`, etc.

### 10. Main Loop State Machine

`main()` MUST:
- Start with `menu.active = true` and `state = GAME_STATE_MENU`
- Only call `GameInit()` from `UIUpdate()` when user starts game, or from auto-start path
- Never call `GameInit()` directly in `main()` before the main loop

### 11. Input Handling Rules

| Action | Input | Implementation |
|--------|-------|----------------|
| Shoot | Left click press | `mouseLeftPressed` (edge trigger) |
| Hold for 1st person | Right click hold | `mouseRightDown` (level trigger) |
| Toggle camera mode | C key | `cameraTogglePressed` |
| Sprint | Shift hold | `shiftPressed` |
| Crouch | Ctrl hold | `ctrlPressed` |
| Jump | Space press | `spacePressed` |

### 12. Camera Mode System

- `baseMode` — persistent mode set by C key toggle (3rd/1st person)
- `mode` — current mode, can be temporarily overridden by right-click hold
- Right-click hold forces `mode = FIRST_PERSON`, release restores `mode = baseMode`
- `CameraUpdate()` lerps `firstPersonBlend` based on `mode`

### 13. Crouch Implementation

- `CameraSetCrouch()` sets `crouchTarget` to 1.0 (crouching) or 0.0 (standing)
- `CameraUpdate()` lerps `crouchAmount` toward `crouchTarget` at 8.0 units/sec
- Crouch offset: `crouchAmount * 0.7f` subtracted from both 3rd-person and 1st-person camera Y

### 14. Jump Physics

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

**This workflow is MANDATORY for every visual update.** See Section 5.1 for the full policy.

After every visual change, you MUST:
1. Build and capture a screenshot using the workflow below
2. Apply the **Judge Protocol** (systematic pixel analysis algorithm in Section 5.2)
3. Compare the result against the reference image
4. Do NOT declare the change complete until the Judge Protocol confirms it matches intent

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

### Judge Protocol (Systematic Pixel Analysis Algorithm)

The **Judge Protocol** is the mandatory verification method for all visual updates.

After capturing a screenshot for a visual change, you MUST apply this algorithm to judge whether the change matches the intended result. Do not rely on visual inspection alone—use the quantitative steps below.

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
