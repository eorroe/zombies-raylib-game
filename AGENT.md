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

## Screenshot Workflow Requirement

### When to Use

You MUST run the headless screenshot workflow for **every change that produces a visible/visual result**, including but not limited to:

- Rendering changes
- Shader changes
- Model/mesh changes
- Material/texture changes
- Animation changes
- Camera/view changes
- UI layout/positioning changes
- Lighting changes
- Color changes
- Any change that affects what the user sees on screen

### When to Skip

You MAY skip the screenshot workflow for changes that do **not** produce a visible result, such as:

- Pure logic bug fixes with no visual side effects
- Refactoring that preserves behavior
- Documentation/comment changes
- Build system changes
- Memory leak fixes
- Performance optimizations that do not change output

If a fix has both a visual component and a non-visual component, treat it as a visual change and run the workflow.

### Rule

After making a visual change and before declaring success, you must:

1. Build
2. Run headless with `ZOMBIE_AUTO_START=1 ZOMBIE_SHOT=/tmp/zombie_test.png ZOMBIE_AUTO_QUIT_MS=5000 xvfb-run -a -s "-screen 0 1280x720x24" ./ZombieShooter`
3. Verify the screenshot exists
4. Analyze the screenshot with Python/PIL or visually inspect it
5. Confirm no blown whites, missing geometry, or other regressions before finalizing

## Headless Testing Workflow

This project supports automated headless testing via Xvfb. Use this workflow whenever you need to verify rendering changes without a physical display.

### Environment Variables

| Variable | Purpose |
|----------|---------|
| `ZOMBIE_AUTO_START=1` | Skips menu and starts gameplay automatically after 30 frames |
| `ZOMBIE_SHOT=/path/to/screenshot.png` | Saves a screenshot at frame 35 to the specified path |
| `ZOMBIE_AUTO_QUIT_MS=5000` | Auto-quits after the specified milliseconds (prevents hangs) |

### Command Template

```bash
cd /workspace/.../sessions/agent_xxx/build
ZOMBIE_AUTO_START=1 ZOMBIE_SHOT=/tmp/zombie_test.png ZOMBIE_AUTO_QUIT_MS=5000 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./ZombieShooter
```

### Step-by-Step Workflow

1. **Build first**: `cd build && make -j"$(nproc)"`
2. **Run headless** with xvfb using the template above
3. **Verify screenshot exists**: `ls -la /tmp/zombie_test.png`
4. **Analyze screenshot** with Python/PIL:
   ```python
   from PIL import Image
   img = Image.open('/tmp/zombie_test.png')
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

### Rules

- Always take a screenshot after rendering changes before declaring success
- If brightness > 200 average, the scene is likely blown out — check PBR output stages
- If non-transparent pixels < 50% of frame, the scene may not be rendering to the backbuffer
- Use `xvfb-run -a` (auto-select display) to avoid conflicts with existing X servers
- Use `-screen 0 1280x720x24` to match the game's window size and color depth
