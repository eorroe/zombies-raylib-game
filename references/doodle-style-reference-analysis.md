# Doodle Style Reference Analysis

## Image Properties
- **Resolution:** 1536×905
- **Aspect ratio:** 1.70 (widescreen game viewport)
- **Mode:** RGB
- **Total pixels:** 1,390,080

## Overall Brightness
- **Bright (>192 luminance):** 86.8%
- **Mid (64–192 luminance):** 12.8%
- **Dark (<64 luminance):** 0.3%
- **Estimated dark ink:** 0.2%

## Dominant Palette
The reference is a warm, light paper sketch:
- **Top colors:** Warm cream/beige with R≈241–244, G≈236–241, B≈224–230
- **Consistent ordering:** R > G > B, with B noticeably lower
- **Average RGB:** (219.4, 219.1, 220.4) — nearly neutral, very slightly cool

## Estimated Composition
- **Paper:** ~78.1%
- **Blue ink:** ~19.5%
- **Dark ink:** ~0.2%

## Spatial Layout
Grid analysis (6×4 cells) shows ink is concentrated in specific regions:

| Region | Paper | Blue Ink | Dark Ink |
|--------|-------|----------|----------|
| Top (sky) | 72–82% | 13–25% | 2–6% |
| Upper mid (action) | 43–86% | 12–55% | 3–9% |
| Lower mid (ground) | 83–95% | 3–15% | 0–2% |
| Bottom (UI) | 64–93% | 9–33% | 1–3% |

## Forms and Objects
- Recognizable 3D scene translated into sketch form
- Buildings, containers, fence lines, and characters read as **outlined volumes** rather than textured surfaces
- Solid fills are rare; form is conveyed through **contour and hatching** rather than tone
- Crosshatching appears on mid-tone surfaces to suggest volume
- Ground/sky areas stay mostly clean paper, helping objects read as drawn elements placed on the page

## Edges
- Edges are the strongest signal in the reference
- **Blue outlines** define object boundaries clearly
- Edge coverage is **selective**, not uniform — only structural edges and silhouette edges are emphasized
- Visible **line weight variation**, with some edges stronger than others

## Faces/Surfaces
- Surfaces are not fully white; they have very subtle tonal variation
- Crosshatching is present but **restrained**
- The paper itself provides the midtone, with hatching adding just enough texture to read as form
- No heavy fill or dense ink coverage on any single surface

## Implications for Implementation
The current shader is missing the core paper-replacement step; it’s inking over a dark PBR render instead of converting the scene to paper first. To match the reference:

1. **Paper color replacement** — convert the 3D scene to bright warm paper before inking
2. **Blue sketch outlines** — extract edges and draw them as blue lines, not dark PBR edges
3. **Selective crosshatching** — apply only to mid-luminance areas with low ink strength
4. **Blue margin line** — the red margin must be changed to blue ink
5. **Clean background** — sky/ground should become clean paper, not remain as dark 3D shading
