# Concept 2 Analysis

## 1. Image Properties

- **Resolution:** 1536x864
- **Aspect ratio:** 1.78
- **Color mode:** RGB
- **Total pixel count:** 1,327,104

## 2. Overall Brightness

- **Dark pixels (<64 luminance):** 61.3%
- **Mid-tone pixels (64–192 luminance):** 38.1%
- **Bright pixels (>192 luminance):** 0.6%
- **Estimated dark ink percentage:** ~61%

## 3. Dominant Palette

**Top 10 colors:**
| RGB | Coverage |
|-----|----------|
| (1, 1, 0) | 1.09% |
| (1, 0, 0) | 0.88% |
| (2, 1, 0) | 0.63% |
| (0, 0, 0) | 0.55% |
| (4, 1, 0) | 0.44% |
| (3, 1, 0) | 0.37% |
| (4, 2, 1) | 0.34% |
| (4, 3, 1) | 0.29% |
| (1, 1, 1) | 0.27% |
| (5, 3, 1) | 0.26% |

- **Average RGB:** (72.0, 48.8, 32.3)
- **Channel ranges:**
  - R: ~0–100, avg ~72
  - G: ~0–80, avg ~49
  - B: ~0–60, avg ~32
- **Color ordering:** R > G > B (very dark warm brown/orange)

## 4. Estimated Composition

- **Dark environment/background:** ~70%
- **Zombie figures:** ~25%
- **Highlights/accents:** ~5%

## 5. Spatial Layout (6x4 grid)

| Cell | Dark | Mid | Bright | Notes |
|------|------|-----|--------|-------|
| (0,0) | 49.8% | 50.2% | 0.0% | Mixed |
| (1,0) | 2.5% | 96.9% | 0.6% | Lit area |
| (2,0) | 0.1% | 97.4% | 2.4% | Bright highlight |
| (3,0) | 0.0% | 100.0% | 0.0% | Clean mid |
| (4,0) | 0.0% | 100.0% | 0.0% | Clean mid |
| (5,0) | 27.1% | 72.9% | 0.0% | Mid-dark |
| (0,1) | 60.8% | 39.2% | 0.0% | Dark |
| (1,1) | 64.0% | 35.8% | 0.2% | Dark |
| (2,1) | 14.8% | 81.2% | 3.9% | Lit |
| (3,1) | 32.8% | 64.7% | 2.5% | Mid |
| (4,1) | 41.8% | 57.5% | 0.7% | Mid-dark |
| (5,1) | 77.9% | 22.0% | 0.1% | Very dark |
| (0,2) | 96.2% | 3.8% | 0.0% | Near black |
| (1,2) | 91.0% | 8.8% | 0.1% | Near black |
| (2,2) | 66.6% | 31.3% | 2.1% | Dark |
| (3,2) | 81.6% | 17.4% | 1.0% | Very dark |
| (4,2) | 86.5% | 13.3% | 0.2% | Near black |
| (5,2) | 96.5% | 3.4% | 0.0% | Near black |
| (0,3) | 97.8% | 2.0% | 0.2% | Near black |
| (1,3) | 97.1% | 2.9% | 0.0% | Near black |
| (2,3) | 95.3% | 4.7% | 0.1% | Near black |
| (3,3) | 96.1% | 3.9% | 0.0% | Near black |
| (4,3) | 96.0% | 4.0% | 0.0% | Near black |
| (5,3) | 98.7% | 1.3% | 0.0% | Near black |

## 6. Forms and Objects

- **Zombie horde** - multiple dark figures in close formation
- **Warehouse/industrial interior** - dark environment
- **Single light source** creating strong rim lighting on zombies
- **Overhead industrial lighting** with warm color temperature
- **Concrete/metal surfaces** - very dark, minimal texture visible
- **Atmospheric smoke/fog** - low visibility

## 7. Edges

- Strong rim lighting on zombie silhouettes
- Most edges are soft due to atmospheric conditions
- Limited edge detail in dark areas
- Light source creates bright halos/edges

## 8. Faces/Surfaces

- Zombies rendered as dark silhouettes with minimal surface detail
- Surfaces are nearly black with subtle warm highlights
- No visible texturing on zombie skin/clothing
- Environment surfaces are dark and flat

## 9. Implications for Implementation

1. **Extreme darkness required:** 60%+ dark pixels. Current implementation is far too bright.
2. **Single key light with warm temperature:** Orange/amber rim lighting from above.
3. **Zombie silhouettes:** Zombies should be nearly black with only rim light visible.
4. **Atmospheric density:** Heavy fog/smoke reducing visibility.
5. **Minimal surface detail:** Don't rely on textures in near-black areas.
6. **High contrast:** Bright highlights against very dark backgrounds.
7. **Industrial lighting:** Overhead single-source lighting model.
