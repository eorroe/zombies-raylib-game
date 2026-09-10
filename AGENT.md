# Agent Instructions: Rendering Rules for doodle-style Branch

This file contains mandatory rules for any agent working on rendering code in this project. Violating these rules will cause visible rendering failures.

## 1. Render Texture Lifecycle

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

## 2. PBR Shader Output Contract

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

## 3. Post-Processing Shader

The post-process shader MUST apply at least gamma correction if the PBR shader does not.

Current post-process output: `pow(col, vec3(1.0 / 2.2))` — keep this.

If you add ACESFilm or other tone mapping to post-process, ensure it's not duplicated in PBR.

## 4. Model Replacement Rules

When replacing any mesh/model:

1. Measure the original bounding box against constants in the header (`TORSO_WIDTH`, `TORSO_HEIGHT`, `HEAD_RADIUS`, `LIMB_RADIUS`, `ARM_*_LEN`, `LEG_*_LEN`)
2. New mesh MUST have equal or greater visual mass
3. Y-position math must be recalculated from actual bone lengths, not hardcoded offsets
4. Original silhouette must not shrink without explicit user approval

## 5. Shader Uniform Updates

When setting shader uniforms in `RendererDrawZombies()` or similar:

- Light colors should use `lightCol[i] * attenuation` (no extra `* 1.2` multiplier)
- Light attenuation should use proper distance falloff: `1.0 / (1.0 + 0.05 * dist + 0.01 * dist * dist)`
- Fire light flicker is ±15-25% — without tonemapping this will cause visible white flash

## 6. Pre-Commit Rendering Checklist

Before committing any rendering change:
- [ ] `BeginTextureMode` has matching `EndTextureMode` + blit
- [ ] PBR shader has tone map AND gamma correction
- [ ] PBR shader has full GGX BRDF if using normal/metallic
- [ ] Ambient is ≤ 0.08 * albedo * ao
- [ ] Model replacements match or exceed original scale
- [ ] Build succeeds with zero errors
- [ ] Screenshot taken and visually verified (no blown whites, scene visible)

## 7. Failure Symptoms and Causes

| Symptom | Cause | Fix |
|---------|-------|-----|
| Screen shows only flat color | `EndTextureMode` missing or blit missing | Add `EndTextureMode()` + `DrawTextureRec` |
| Scene visible but blown-out white | Missing tonemap or gamma in PBR | Add Reinhard + gamma to PBR shader |
| Visible white flash pulsing | Fire lights + unconstrained HDR | Add tonemap, or reduce light intensity |
| Zombies invisible | Model scale too small | Compare against original constants |
| HUD invisible | Drawn inside texture mode | Draw HUD after blit, on backbuffer |
| Zombie heads at wrong positions | `GetWorldToScreen` called inside texture mode | Call after `EndTextureMode`, on backbuffer |
