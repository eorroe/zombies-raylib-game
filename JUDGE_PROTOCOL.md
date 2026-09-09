# Judge Protocol

## Overview

This document defines the Judge Protocol for evaluating visual quality and ensuring the final output meets concept art standards. The protocol enforces a rigorous self-assessment loop before any external review, with tiered judging criteria and clear exit conditions.

## Self-Assessment Loop

Before submitting any work for external review:

1. Review the candidate output yourself
2. Ensure it actually achieves the stated goals
3. Do not submit half-baked work
4. Step back and look at the output and concept side-by-side
5. Log an honest assessment of whether it is judge-ready
6. Only submit if you are confident you have significantly improved the score
7. Be rigorous, objective, and transparent in self-assessment
8. Look at every pixel and detail
9. Even small touches make a big difference
10. Check for: missing/incorrect objects, wrong scale, perspective, positioning
11. Check for: rendering glitches, flat untextured surfaces, ugly lighting, poor contrast, speckles, ugly shadows
12. Scan surface by surface, object by object, audit everything

## Judge Prompt

When submitting a screenshot to the judge, use the following prompt:

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

## Exit Criteria

- **Score >= 8 and target FPS acceptable**: Done! Show the user the latest screenshot and ask if they want more iterations.
- **Score >= 8 but target FPS unacceptable**: Optimize, aiming for lossless wins first, then optimizations that have minimal visual impact. Re-judge after optimizations to ensure you didn't regress visuals.
- **Stall approaching**: The best score hasn't improved by a full point in 2 rounds, or the judge has named the same gap 3 times. Stop making incremental tweaks. Step back and assess the whole frame against the concept: what about the approach is capping the score? Then make a big, structural change in one round: swap the asset strategy (sculpt in Blender, pull real models/textures/HDRIs from an asset library), rewrite the lighting model, rebuild the composition, change the camera. Self-check the result before it goes to the judge, since big changes break things. Only do the same-old parameter tuning if you can articulate why it would move the score this time when it didn't last time. Do not tunnel vision on incremental wins when the judge is telling you that you're completely off base.
- **Stalled**: You've already tried at least one big structural change as above, and the best score still hasn't improved in 3 rounds, and the judge is either blocking you over extremely nitpicky things or asking for improvements that are intractable (e.g. it wants raytracing but you're on a cheap laptop with no GPU). Stop and tell the user why you think you're blocked, and give options for what to do next.
- **None of the above**: Address all or most of the judge's heavy-hitting gaps in this round, not just the top one. Rounds are expensive; make each one count. Prioritize the gaps that move the needle most relative to the concept (often things like improving lighting, textures, or sculpting fine details on meshes). Only revert if the score dropped by a full point or more: small dips are judge noise, and reverting a whole round throws out the good changes with the bad. If a specific change clearly caused a regression, undo just that change. Loop back around.

## Visual Quality Requirements

Do not be lazy and resort to simple shapes or procedural assets for key environmental details like scenery, flooring, buildings, etc. These will look blocky, shiny, flat, and fake. The tiny details and texturing matter and require custom sculpting.

If you have an image generation tool, use it for textures, normal maps, skyboxes, etc, to enhance the visuals. This looks better and is faster than procedurally generated ones. Do not settle for plain, flat, procedural looks unless the art style requires it.

## Target Resolution

When submitting screenshots to the judge, target the same resolution and aspect ratio as the concept art, so the comparison is fair.

## Subagent Judge

Judging should ideally be done by a fresh subagent with a clean context each time, to keep it objective and cheap.

The judge should be given the latest live screenshot, the concept image, and (from round 2 on) the previous round's screenshot and verdict.

## Process Integration

This protocol applies to all visual development work, including:
- Environment and terrain rendering
- Character and zombie models
- Lighting and atmosphere
- Textures and materials
- Post-processing effects
- UI/HUD design

Every major visual iteration should go through this self-assessment loop before being committed or presented.
