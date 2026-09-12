# Reference Analysis Workflow

## Purpose

This document defines the exact process to follow when analyzing any reference image for style, color, composition, or implementation guidance. Every reference image analysis MUST produce a corresponding markdown file in the `references/` directory.

## Trigger

Use this workflow whenever:
- A new reference image is added to `references/`
- An existing reference image is updated
- A new style guide or target rendering is introduced
- The team needs to extract implementation guidance from visual reference material

## Output

For every reference image analyzed, create a markdown file in `references/` named after the reference image.

**Example:**
- Reference: `references/doodle-style-reference.png`
- Analysis: `references/doodle-style-reference-analysis.md`

## Required Analysis Sections

Every reference analysis document MUST include:

1. **Image Properties**
   - Resolution
   - Aspect ratio
   - Color mode
   - Total pixel count

2. **Overall Brightness**
   - Percentage of dark pixels (<64 luminance)
   - Percentage of mid-tone pixels (64–192 luminance)
   - Percentage of bright pixels (>192 luminance)
   - Estimated dark ink percentage if applicable

3. **Dominant Palette**
   - Top N dominant colors with RGB values and coverage percentages
   - Average RGB values
   - Color channel ranges (min, max, average per channel)
   - Color ordering patterns (e.g., R > G > B)

4. **Estimated Composition**
   - Paper/background percentage
   - Primary ink color percentage
   - Secondary ink color percentage
   - Dark ink percentage

5. **Spatial Layout**
   - Divide image into a grid (minimum 4×3 or 6×4)
   - For each cell, report paper/ink/dark percentages
   - Identify where ink is concentrated vs. clean

6. **Forms and Objects**
   - List recognizable objects or scene elements
   - Describe how forms are rendered (outlined, filled, textured, etc.)
   - Note any repeating elements or patterns

7. **Edges**
   - Describe edge treatment (color, width, consistency)
   - Note if edges are selective or universal
   - Identify any edge weight variation

8. **Faces/Surfaces**
   - Describe surface treatment (flat, textured, hatched, etc.)
   - Note tonal variation approach
   - Identify any fill patterns or hatching styles

9. **Implications for Implementation**
   - List concrete changes needed to match the reference
   - Prioritize by impact
   - Note any missing capabilities in current implementation

## Analysis Process

### Step 1: Load and Inspect Basic Properties

```python
from PIL import Image
from collections import Counter

img = Image.open("path/to/reference.png").convert("RGB")
width, height = img.size
pixels = list(img.getdata())
total = len(pixels)
```

Record:
- Width × height
- Aspect ratio (width / height)
- Color mode
- Total pixel count

### Step 2: Calculate Average Color and Luminance Distribution

```python
avg_r = sum(p[0] for p in pixels) / total
avg_g = sum(p[1] for p in pixels) / total
avg_b = sum(p[2] for p in pixels) / total

dark = sum(1 for p in pixels if (p[0]+p[1]+p[2])/3 < 64)
mid = sum(1 for p in pixels if 64 <= (p[0]+p[1]+p[2])/3 < 192)
bright = sum(1 for p in pixels if (p[0]+p[1]+p[2])/3 >= 192)
```

Report percentages for each luminance band.

### Step 3: Identify Dominant Colors

```python
color_counts = Counter(pixels)
top_colors = color_counts.most_common(20)
```

For each top color:
- Report RGB values
- Report pixel count and percentage
- Note any color patterns (warm vs cool, saturation level)

Also report:
- R, G, B channel ranges (min, max, average)
- Any consistent channel ordering patterns

### Step 4: Estimate Composition

Define classification rules based on the reference style:

```python
# Example for paper sketch style
paper_pixels = sum(1 for p in pixels if luminance > 0.75 and p[0] > p[1] and p[0] > p[2])
blue_ink_pixels = sum(1 for p in pixels if p[2] > p[0] and p[2] > p[1] and p[2] > 100)
dark_ink_pixels = sum(1 for p in pixels if luminance < 0.2)
```

Adjust rules to match the specific reference style. Report percentages.

### Step 5: Spatial Grid Analysis

Divide image into a grid (minimum 4×3, prefer 6×4 for detailed analysis):

```python
grid_cols, grid_rows = 6, 4
cell_w, cell_h = width // grid_cols, height // grid_rows

for row in range(grid_rows):
    for col in range(grid_cols):
        x1, y1 = col * cell_w, row * cell_h
        crop = img.crop((x1, y1, x1 + cell_w, y1 + cell_h))
        cell_pixels = list(crop.getdata())
        # Analyze cell...
```

For each cell, report:
- Paper percentage
- Primary ink percentage
- Dark ink percentage
- Any notable observations

### Step 6: Edge and Texture Analysis

Analyze edge density and character:

```python
edges = 0
blue_edges = 0
for i in range(1, len(pixels) - 1):
    r1, g1, b1 = pixels[i-1]
    r2, g2, b2 = pixels[i+1]
    diff = abs(r1-r2) + abs(g1-g2) + abs(b1-b2)
    if diff > threshold:  # threshold depends on style
        edges += 1
        if is_blue_edge(r1, g1, b1, r2, g2, b2):
            blue_edges += 1
```

Report:
- Total edge-like transitions
- Percentage of edges that are ink-colored
- Edge character (sharp, soft, variable)

### Step 7: Form and Object Identification

Manually inspect the image and document:
- Recognizable 3D objects or scene elements
- How each object is rendered (outline only, filled, hatched, etc.)
- Spatial relationships between objects
- Any text, UI elements, or overlays

### Step 8: Implementation Implications

Based on the analysis, document:
- What the current implementation is missing
- What needs to change in shaders, materials, or rendering pipeline
- Priority of changes (high/medium/low)
- Any technical constraints or limitations discovered

## Documentation Requirements

The analysis markdown file MUST:
1. Be placed in `references/` directory
2. Be named `{reference-name}-analysis.md` (e.g., `doodle-style-reference-analysis.md`)
3. Include all 9 required sections listed above
4. Be written in clear, technical Markdown
5. Include code snippets where analysis logic is non-obvious
6. Be committed and pushed to `doodle-style` branch

## Quality Checklist

Before finalizing the analysis document, verify:

- [ ] All 9 required sections are present
- [ ] Numerical data is included (percentages, counts, averages)
- [ ] Grid analysis covers the full image
- [ ] Implementation implications are concrete and actionable
- [ ] File is saved in `references/` directory
- [ ] File follows naming convention
- [ ] Changes are committed to `doodle-style` branch
- [ ] Changes are pushed to `origin/doodle-style`

## Example Usage

```bash
# 1. Analyze the reference image
python3 analyze_reference.py references/my-style-reference.png

# 2. Create analysis document
# File: references/my-style-reference-analysis.md

# 3. Commit and push
git add references/my-style-reference-analysis.md
git commit -m "docs(reference): add analysis for my-style-reference.png"
git push origin doodle-style
```

## Notes

- This workflow applies to ANY reference image, not just doodle style
- Adapt the analysis rules (color classification, edge detection, etc.) to the specific style being analyzed
- When in doubt, include more detail rather than less
- The analysis should be useful for both artistic direction and technical implementation
