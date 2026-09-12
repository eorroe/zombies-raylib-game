# Concept 3 Analysis

## 1. Image Properties

- **Resolution:** 1536x864
- **Aspect ratio:** 1.78
- **Color mode:** RGB
- **Total pixel count:** 1,327,104

## 2. Overall Brightness

- **Dark pixels (<64 luminance):** 36.0%
- **Mid-tone pixels (64–192 luminance):** 61.1%
- **Bright pixels (>192 luminance):** 2.8%
- **Estimated dark ink percentage:** ~36%

## 3. Dominant Palette

**Top 10 colors:**
| RGB | Coverage |
|-----|----------|
| (59, 67, 69) | 0.06% |
| (57, 65, 67) | 0.06% |
| (50, 58, 60) | 0.06% |
| (51, 59, 61) | 0.06% |
| (55, 63, 65) | 0.06% |
| (54, 62, 64) | 0.06% |
| (49, 57, 59) | 0.06% |
| (58, 66, 68) | 0.06% |
| (52, 60, 62) | 0.06% |
| (56, 64, 66) | 0.06% |

- **Average RGB:** (93.2, 89.2, 83.3)
- **Channel ranges:**
  - R: ~0–175, avg ~93
  - G: ~0–165, avg ~89
  - B: ~0–150, avg ~83
- **Color ordering:** R > G > B (dark gray with slight warm bias)

## 4. Estimated Composition

- **Warehouse/interior background:** ~50%
- **Zombie figures:** ~30%
- **Foreground elements:** ~20%

## 5. Spatial Layout (6x4 grid)

| Cell | Dark | Mid | Bright | Notes |
|------|------|-----|--------|-------|
| (0,0) | 32.4% | 67.4% | 0.2% | Mid-dark |
| (1,0) | 30.9% | 69.1% | 0.0% | Mid-dark |
| (2,0) | 17.8% | 80.1% | 2.1% | Lit mid |
| (3,0) | 25.7% | 63.4% | 10.9% | Highlight |
| (4,0) | 29.9% | 70.0% | 0.1% | Mid |
| (5,0) | 24.1% | 75.9% | 0.0% | Mid |
| (0,1) | 52.6% | 46.5% | 0.9% | Dark |
| (1,1) | 21.2% | 66.7% | 12.1% | Lit |
| (2,1) | 27.3% | 65.8% | 6.9% | Mid |
| (3,1) | 40.7% | 55.5% | 3.8% | Mid-dark |
| (4,1) | 34.6% | 59.4% | 6.0% | Mid |
| (5,1) | 50.1% | 49.4% | 0.5% | Dark |
| (0,2) | 39.6% | 60.4% | 0.0% | Mid-dark |
| (1,2) | 45.6% | 50.5% | 3.9% | Dark |
| (2,2) | 10.9% | 75.8% | 13.3% | Bright |
| (3,2) | 17.2% | 80.2% | 2.6% | Lit |
| (4,2) | 56.8% | 42.6% | 0.6% | Dark |
| (5,2) | 58.2% | 41.5% | 0.4% | Dark |
| (0,3) | 40.5% | 59.5% | 0.0% | Mid-dark |
| (1,3) | 60.9% | 39.1% | 0.0% | Dark |
| (2,3) | 5.9% | 93.9% | 0.2% | Very lit |
| (3,3) | 16.1% | 83.2% | 0.7% | Lit |
| (4,3) | 63.2% | 35.7% | 1.1% | Dark |
| (5,3) | 62.9% | 35.9% | 1.2% | Dark |

## 6. Forms and Objects

- **Warehouse interior** with industrial elements
- **Zombie figures** in mid-ground and background
- **Overhead lighting fixtures** creating pools of light
- **Structural columns/beams**
- **Pallets/crates** on floor
- **Atmospheric haze** throughout

## 7. Edges

- Soft edges due to atmospheric haze
- Zombie edges are slightly more defined but still soft
- Lighting fixtures create soft glow edges
- No hard line work; forms defined by value

## 8. Faces/Surfaces

- Dark gray concrete/metal surfaces
- Subtle surface variation through noise
- Zombie surfaces are dark with minimal detail
- Light pools create surface definition

## 9. Implications for Implementation

1. **Moderate darkness:** ~36% dark pixels. Still darker than current implementation.
2. **Warehouse setting:** Add interior structures, columns, overhead lights.
3. **Zombie placement:** Figures in mid-ground under lighting fixtures.
4. **Lighting pools:** Create distinct pools of light on floor from above.
5. **Atmospheric haze:** Medium-density fog for depth.
6. **Industrial materials:** Dark concrete, metal, wood pallets.
7. **Color temperature:** Slightly cool/warm mix in mid-tones.
