#include "texture.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

float FastHashNoise(int x, int y) {
    int n = x * 374761393 + y * 668265263;
    n = (n ^ (n >> 13)) * 1274126177;
    n = n ^ (n >> 16);
    return (n & 0x7fffffff) / (float)0x7fffffff;
}

float SmoothValueNoise(int x, int y, float frequency) {
    int ix = (int)(x * frequency);
    int iy = (int)(y * frequency);
    float fx = x * frequency - ix;
    float fy = y * frequency - iy;
    float tx = fx * fx * (3.0f - 2.0f * fx);
    float ty = fy * fy * (3.0f - 2.0f * fy);
    float v00 = FastHashNoise(ix, iy);
    float v10 = FastHashNoise(ix + 1, iy);
    float v01 = FastHashNoise(ix, iy + 1);
    float v11 = FastHashNoise(ix + 1, iy + 1);
    return (v00 * (1.0f - tx) + v10 * tx) * (1.0f - ty) +
           (v01 * (1.0f - tx) + v11 * tx) * ty;
}

float FractalNoise(int x, int y, int octaves, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxAmplitude = 0.0f;
    for (int i = 0; i < octaves; i++) {
        total += SmoothValueNoise(x, y, frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    return total / maxAmplitude;
}

static Image GenerateNormalMapFromHeightMap(Image heightMap, float strength) {
    int w = heightMap.width;
    int h = heightMap.height;
    Image normalMap = GenImageColor(w, h, BLANK);
    unsigned char *src = (unsigned char *)heightMap.data;
    unsigned char *dst = (unsigned char *)normalMap.data;
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            float tl = src[((y - 1) * w + (x - 1)) * 4];
            float tc = src[((y - 1) * w + x) * 4];
            float tr = src[((y - 1) * w + (x + 1)) * 4];
            float ml = src[(y * w + (x - 1)) * 4];
            float mr = src[(y * w + (x + 1)) * 4];
            float bl = src[((y + 1) * w + (x - 1)) * 4];
            float bc = src[((y + 1) * w + x) * 4];
            float br = src[((y + 1) * w + (x + 1)) * 4];
            float dx = (tr + 2.0f * mr + br) - (tl + 2.0f * ml + bl);
            float dy = (bl + 2.0f * bc + br) - (tl + 2.0f * tc + tr);
            float nx = -dx * strength / 255.0f;
            float ny = -dy * strength / 255.0f;
            float nz = 1.0f;
            float len = sqrtf(nx * nx + ny * ny + nz * nz);
            if (len > 0.00001f) { nx /= len; ny /= len; nz /= len; }
            int idx = (y * w + x) * 4;
            dst[idx + 0] = (unsigned char)((nx * 0.5f + 0.5f) * 255);
            dst[idx + 1] = (unsigned char)((ny * 0.5f + 0.5f) * 255);
            dst[idx + 2] = (unsigned char)((nz * 0.5f + 0.5f) * 255);
            dst[idx + 3] = 255;
        }
    }
    return normalMap;
}

static Image ExtractGrayscaleHeightMap(Image img) {
    int w = img.width;
    int h = img.height;
    Image heightMap = GenImageColor(w, h, BLANK);
    unsigned char *src = (unsigned char *)img.data;
    unsigned char *dst = (unsigned char *)heightMap.data;
    for (int i = 0; i < w * h; i++) {
        float lum = 0.299f * src[i * 4] + 0.587f * src[i * 4 + 1] + 0.114f * src[i * 4 + 2];
        unsigned char v = (unsigned char)fminf(255.0f, fmaxf(0.0f, lum));
        dst[i * 4] = dst[i * 4 + 1] = dst[i * 4 + 2] = v;
        dst[i * 4 + 3] = 255;
    }
    return heightMap;
}

static void AddNoiseLayer(Image *img, float scale, float amplitude, int octaves) {
    int w = img->width;
    int h = img->height;
    unsigned char *data = (unsigned char *)img->data;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float n = FractalNoise(x, y, octaves, 0.5f) * amplitude;
            int idx = (y * w + x) * 4;
            for (int c = 0; c < 3; c++) {
                int val = data[idx + c] + (int)n;
                if (val > 255) val = 255;
                if (val < 0) val = 0;
                data[idx + c] = (unsigned char)val;
            }
        }
    }
}

  static Image GenerateZombieSkin(int width, int height) {
      Image img = GenImageColor(width, height, (Color){ 80, 120, 170, 255 });
      unsigned char *data = (unsigned char *)img.data;
      for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
              float baseNoise = FractalNoise(x, y, 6, 0.5f);
              float poreNoise = FractalNoise(x + 100, y + 100, 4, 0.6f);
              int idx = (y * width + x) * 4;
              int r = 80 + (int)(baseNoise * 60 + poreNoise * 10);
              int g = 120 + (int)(baseNoise * 30 - poreNoise * 10);
              int b = 170 + (int)(baseNoise * 40 - poreNoise * 10);
              if (r > 150) r = 150; if (r < 60) r = 60;
              if (g > 150) g = 150; if (g < 60) g = 60;
              if (b > 150) b = 150; if (b < 60) b = 60;
              data[idx + 0] = (unsigned char)r;
              data[idx + 1] = (unsigned char)g;
              data[idx + 2] = (unsigned char)b;
              data[idx + 3] = 255;
          }
      }
     for (int i = 0; i < 3000; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 2 + rand() % 8;
         Color veinColor = { 40, 60, 100, 200 };
         ImageDrawCircle(&img, cx, cy, r, veinColor);
     }
     for (int i = 0; i < 150; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 4 + rand() % 12;
         Color dirtColor = { 50, 60, 80, 180 };
         ImageDrawCircle(&img, cx, cy, r, dirtColor);
     }
     for (int i = 0; i < 80; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 3 + rand() % 10;
         Color woundColor = { 30, 40, 70, 220 };
         ImageDrawCircle(&img, cx, cy, r, woundColor);
     }
     return img;
 }

 static Image GenerateConcreteTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 140, 150, 165, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n1 = FractalNoise(x, y, 5, 0.5f);
             float n2 = FractalNoise(x + 500, y + 500, 3, 0.6f);
             float n3 = FractalNoise(x + 1000, y + 1000, 4, 0.4f);
             int idx = (y * width + x) * 4;
             int shade = (int)(140 + n1 * 20 + n2 * 10 + n3 * 5);
             if (shade > 160) shade = 160;
             if (shade < 130) shade = 130;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)(shade + 10);
             data[idx + 2] = (unsigned char)(shade + 25);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 60; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 20 + rand() % 100;
         int angle = rand() % 360;
         int x2 = x1 + (int)(cosf(angle * DEG2RAD) * len);
         int y2 = y1 + (int)(sinf(angle * DEG2RAD) * len);
         Color crack = { 80, 90, 100, 180 };
         ImageDrawLine(&img, x1, y1, x2, y2, crack);
     }
     for (int i = 0; i < 500; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 3 + rand() % 20;
         Color stain = { 110, 120, 130, 120 };
         ImageDrawCircle(&img, cx, cy, r, stain);
     }
     for (int i = 0; i < 800; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 1 + rand() % 4;
         Color spot = { 140, 150, 165, 150 };
         ImageDrawCircle(&img, cx, cy, r, spot);
     }
     for (int i = 0; i < 30; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 30 + rand() % 120;
         Color scratch = { 110, 120, 130, 140 };
         ImageDrawLine(&img, x1, y1, x1 + len, y1 + (rand() % 10 - 5), scratch);
     }
     return img;
 }

 static Image GenerateMetalTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 80, 90, 100, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n = FractalNoise(x, y, 4, 0.5f);
             float n2 = FractalNoise(x + 200, y + 200, 3, 0.3f);
             int idx = (y * width + x) * 4;
             int shade = (int)(80 + n * 25 + n2 * 10);
             if (shade > 110) shade = 110;
             if (shade < 70) shade = 70;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)(shade + 10);
             data[idx + 2] = (unsigned char)(shade + 20);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 250; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 10 + rand() % 80;
         int x2 = x1 + len;
         int y2 = y1 + (rand() % 3 - 1);
         Color scratch = { 50, 55, 65, 120 };
         ImageDrawLine(&img, x1, y1, x2, y2, scratch);
     }
     for (int i = 0; i < 60; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 3 + rand() % 12;
         Color rust = { 60, 50, 80, 160 };
         ImageDrawCircle(&img, cx, cy, r, rust);
     }
     for (int i = 0; i < 300; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 2 + rand() % 8;
         Color wear = { 70, 75, 80, 140 };
         ImageDrawCircle(&img, cx, cy, r, wear);
     }
     return img;
 }

 static Image GenerateDarkMetalTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 40, 45, 55, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n = FractalNoise(x + 300, y + 300, 3, 0.5f);
             int idx = (y * width + x) * 4;
             int shade = (int)(40 + n * 15);
             if (shade > 50) shade = 50;
             if (shade < 30) shade = 30;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)(shade + 5);
             data[idx + 2] = (unsigned char)(shade + 15);
             data[idx + 3] = 255;
         }
     }
     return img;
 }

 static Image GenerateGripTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 45, 38, 55, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n = FractalNoise(x, y, 3, 0.5f);
             int idx = (y * width + x) * 4;
             int shade = (int)(45 + n * 20);
             if (shade > 65) shade = 65;
             if (shade < 35) shade = 35;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)(shade - 7);
             data[idx + 2] = (unsigned char)(shade + 10);
             data[idx + 3] = 255;
         }
     }
     return img;
 }

 static Image GenerateBloodTexture(int width, int height) {
     Image img = GenImageColor(width, height, BLANK);
     for (int i = 0; i < 2000; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 2 + rand() % 25;
         int alpha = 180 + rand() % 75;
         Color splatter = { 20, 30, 60, (unsigned char)alpha };
         ImageDrawCircle(&img, cx, cy, r, splatter);
     }
     for (int i = 0; i < 500; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 5 + rand() % 40;
         int angle = 90 + (rand() % 60 - 30);
         int x2 = x1 + (int)(cosf(angle * DEG2RAD) * len);
         int y2 = y1 + (int)(sinf(angle * DEG2RAD) * len);
         int thick = 1 + rand() % 3;
         Color drip = { 25, 35, 65, 220 };
         ImageDrawLineEx(&img, (Vector2){ x1, y1 }, (Vector2){ x2, y2 }, thick, drip);
     }
     for (int i = 0; i < 1000; i++) {
         int x = rand() % width;
         int y = rand() % height;
         int r = 1 + rand() % 4;
         Color drop = { 30, 40, 70, 200 + rand() % 55 };
         ImageDrawCircle(&img, x, y, r, drop);
     }
     return img;
 }

 static Image GenerateUniformTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 60, 100, 160, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n = FractalNoise(x, y, 4, 0.5f);
             int idx = (y * width + x) * 4;
             int shade = (int)(100 + n * 20);
             if (shade > 120) shade = 120;
             if (shade < 50) shade = 50;
             data[idx + 0] = (unsigned char)(shade - 40);
             data[idx + 1] = (unsigned char)(shade);
             data[idx + 2] = (unsigned char)(shade + 60);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 100; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 4 + rand() % 10;
         ImageDrawCircle(&img, cx, cy, r, (Color){ 30, 50, 90, 220 });
     }
     return img;
 }

 static Image GenerateAsphaltTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 35, 40, 50, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n1 = FractalNoise(x, y, 5, 0.5f);
             float n2 = FractalNoise(x + 200, y + 200, 4, 0.6f);
             int idx = (y * width + x) * 4;
             int shade = (int)(40 + n1 * 15 + n2 * 10);
             if (shade > 55) shade = 55;
             if (shade < 30) shade = 30;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)(shade);
             data[idx + 2] = (unsigned char)(shade + 10);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 200; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 1 + rand() % 3;
         Color gravel = { 50, 55, 60, 160 };
         ImageDrawCircle(&img, cx, cy, r, gravel);
     }
     for (int i = 0; i < 15; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 20 + rand() % 60;
         int x2 = x1 + len;
         int y2 = y1 + (rand() % 4 - 2);
         Color crack = { 20, 25, 35, 180 };
         ImageDrawLine(&img, x1, y1, x2, y2, crack);
     }
     return img;
 }

 static Image GenerateBrickTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 80, 90, 100, 255 });
     int brickW = width / 6;
     int brickH = height / 8;
     int mortarSize = 4;
     for (int row = 0; row < 8; row++) {
         int offset = (row % 2) * (brickW / 2);
         for (int col = -1; col < 7; col++) {
             int bx = col * brickW + offset;
             int by = row * brickH;
             int r = 90 + rand() % 30;
             int g = 100 + rand() % 20;
             int b = 110 + rand() % 20;
             Color brick = { (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
             ImageDrawRectangle(&img, bx + mortarSize, by + mortarSize,
                                brickW - mortarSize * 2, brickH - mortarSize * 2, brick);
             for (int py = by + mortarSize; py < by + brickH - mortarSize; py += 3) {
                 for (int px = bx + mortarSize; px < bx + brickW - mortarSize; px += 3) {
                     float n = FractalNoise(px, py, 3, 0.5f);
                     int shade = (int)(n * 30);
                     ImageDrawPixel(&img, px, py, (Color){
                         (unsigned char)(r - shade),
                         (unsigned char)(g - shade),
                         (unsigned char)(b - shade), 255
                     });
                 }
             }
         }
     }
     return img;
 }

 static Image GenerateCamoTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 50, 70, 110, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n1 = FractalNoise(x, y, 4, 0.5f);
             float n2 = FractalNoise(x + 100, y + 100, 3, 0.6f);
             int idx = (y * width + x) * 4;
             int base = (int)(70 + n1 * 25 + n2 * 15);
             if (base > 95) base = 95;
             if (base < 40) base = 40;
             int r = (int)(base - 20);
             int g = (int)(base);
             int b = (int)(base + 40);
             if (r > 255) r = 255;
             if (b < 30) b = 30;
             data[idx + 0] = (unsigned char)r;
             data[idx + 1] = (unsigned char)g;
             data[idx + 2] = (unsigned char)b;
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 60; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int w = 20 + rand() % 80;
         int h = 10 + rand() % 40;
         int angle = rand() % 360;
         Color patch1 = { 30, 45, 80, 200 };
         Color patch2 = { 70, 80, 100, 180 };
         ImageDrawRectangle(&img, cx, cy, w, h, i % 2 == 0 ? patch1 : patch2);
     }
     for (int i = 0; i < 200; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 1 + rand() % 3;
         ImageDrawCircle(&img, cx, cy, r, (Color){ 60, 75, 40, 150 });
     }
     return img;
 }

 static Image GenerateFenceTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 40, 50, 70, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n = FractalNoise(x, y, 3, 0.5f);
             float n2 = FractalNoise(x + 300, y + 300, 2, 0.4f);
             int idx = (y * width + x) * 4;
             int shade = (int)(50 + n * 15 + n2 * 10);
             if (shade > 60) shade = 60;
             if (shade < 30) shade = 30;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)shade;
             data[idx + 2] = (unsigned char)(shade + 20);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 120; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 10 + rand() % 60;
         Color scratch = { 25, 30, 40, 140 };
         ImageDrawLine(&img, x1, y1, x1 + len, y1 + (rand() % 5 - 2), scratch);
     }
     for (int i = 0; i < 50; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 2 + rand() % 10;
         Color rust = { 50, 40, 70, 160 };
         ImageDrawCircle(&img, cx, cy, r, rust);
     }
     for (int i = 0; i < 200; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 1 + rand() % 3;
         Color wear = { 60, 65, 70, 130 };
         ImageDrawCircle(&img, cx, cy, r, wear);
     }
     return img;
 }

 static Image GenerateContainerTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 60, 80, 110, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float n = FractalNoise(x + 300, y + 300, 4, 0.5f);
             int idx = (y * width + x) * 4;
             int shade = (int)(80 + n * 30);
             if (shade > 120) shade = 120;
             if (shade < 50) shade = 50;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)shade;
             data[idx + 2] = (unsigned char)(shade + 30);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 40; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 30 + rand() % 120;
         Color peel = { 40, 55, 80, 180 };
         ImageDrawLine(&img, x1, y1, x1 + len, y1 + (rand() % 10 - 5), peel);
     }
     for (int i = 0; i < 80; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 2 + rand() % 15;
         Color rust = { 70, 60, 100, 200 };
         ImageDrawCircle(&img, cx, cy, r, rust);
     }
     for (int i = 0; i < 40; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 5 + rand() % 25;
         Color dent = { 35, 45, 65, 140 };
         ImageDrawCircle(&img, cx, cy, r, dent);
     }
     for (int i = 0; i < 20; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 20 + rand() % 80;
         Color scratch = { 50, 55, 65, 160 };
         ImageDrawLine(&img, x1, y1, x1 + len, y1 + (rand() % 8 - 4), scratch);
     }
     return img;
 }

 static Image GenerateWoodTexture(int width, int height) {
     Image img = GenImageColor(width, height, (Color){ 100, 85, 70, 255 });
     unsigned char *data = (unsigned char *)img.data;
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             float grain = sinf(x * 0.08f + FractalNoise(x, y, 4, 0.5f) * 3.5f) * 0.5f + 0.5f;
             float n = FractalNoise(x + 100, y + 100, 3, 0.5f);
             int idx = (y * width + x) * 4;
             int shade = (int)(105 + grain * 15 + n * 10);
             if (shade > 120) shade = 120;
             if (shade < 90) shade = 90;
             data[idx + 0] = (unsigned char)shade;
             data[idx + 1] = (unsigned char)(shade - 15);
             data[idx + 2] = (unsigned char)(shade - 35);
             data[idx + 3] = 255;
         }
     }
     for (int i = 0; i < 25; i++) {
         int cx = rand() % width;
         int cy = rand() % height;
         int r = 4 + rand() % 18;
         Color knot = { 60, 50, 40, 210 };
         ImageDrawCircle(&img, cx, cy, r, knot);
     }
     for (int i = 0; i < 30; i++) {
         int x1 = rand() % width;
         int y1 = rand() % height;
         int len = 10 + rand() % 40;
         Color crack = { 60, 45, 35, 160 };
         ImageDrawLine(&img, x1, y1, x1 + len, y1 + (rand() % 6 - 3), crack);
     }
     return img;
 }

static Image GenerateFireGlowTexture(int width, int height) {
    Image img = GenImageColor(width, height, BLANK);
    unsigned char *data = (unsigned char *)img.data;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = (x - width / 2) / (float)(width / 2);
            float dy = (y - height / 2) / (float)(height / 2);
            float dist = sqrtf(dx * dx + dy * dy);
            float n = FractalNoise(x, y, 4, 0.5f) * 0.3f;
            float intensity = 1.0f - dist + n;
            if (intensity > 1.0f) intensity = 1.0f;
            if (intensity < 0.0f) intensity = 0.0f;
             int r = (int)(200 * intensity);
             int g = (int)(180 * intensity);
             int b = (int)(255 * intensity);
             int a = (int)(255 * intensity);
            int idx = (y * width + x) * 4;
            data[idx + 0] = (unsigned char)(r > 255 ? 255 : r);
            data[idx + 1] = (unsigned char)(g > 255 ? 255 : g);
            data[idx + 2] = (unsigned char)(b > 255 ? 255 : b);
            data[idx + 3] = (unsigned char)(a > 255 ? 255 : a);
        }
    }
    return img;
}

void TextureGenerate(ProceduralTextures *textures) {
    Image zombieSkinImg = GenerateZombieSkin(1024, 1024);
    Image zombieSkinHeight = ExtractGrayscaleHeightMap(zombieSkinImg);
    Image zombieSkinNormal = GenerateNormalMapFromHeightMap(zombieSkinHeight, 2.0f);
    textures->zombieSkin = LoadTextureFromImage(zombieSkinImg);
    textures->zombieSkinNormal = LoadTextureFromImage(zombieSkinNormal);
    UnloadImage(zombieSkinImg);
    UnloadImage(zombieSkinNormal);

    Image zombieHeadImg = GenerateZombieSkin(512, 512);
    textures->defaultZombieHead = LoadTextureFromImage(zombieHeadImg);
    UnloadImage(zombieHeadImg);

    textures->zombieShirt = LoadTextureFromImage(GenerateUniformTexture(512, 512));
    textures->zombiePants = LoadTextureFromImage(GenerateUniformTexture(512, 512));

    textures->bloodDecal = LoadTextureFromImage(GenerateBloodTexture(512, 512));

    Image concreteImg = GenerateConcreteTexture(1024, 1024);
    Image concreteHeight = ExtractGrayscaleHeightMap(concreteImg);
    Image concreteNormal = GenerateNormalMapFromHeightMap(concreteHeight, 3.0f);
    textures->concrete = LoadTextureFromImage(concreteImg);
    textures->concreteNormal = LoadTextureFromImage(concreteNormal);
    UnloadImage(concreteImg);
    UnloadImage(concreteNormal);

    Image metalImg = GenerateMetalTexture(512, 512);
    Image metalHeight = ExtractGrayscaleHeightMap(metalImg);
    Image metalNormal = GenerateNormalMapFromHeightMap(metalHeight, 2.5f);
    textures->metal = LoadTextureFromImage(metalImg);
    textures->metalNormal = LoadTextureFromImage(metalNormal);
    UnloadImage(metalImg);
    UnloadImage(metalNormal);

    textures->darkMetal = LoadTextureFromImage(GenerateDarkMetalTexture(512, 512));
    textures->grip = LoadTextureFromImage(GenerateGripTexture(512, 512));

    Image barrelImg = GenerateMetalTexture(256, 256);
    Image barrelHeight = ExtractGrayscaleHeightMap(barrelImg);
    Image barrelNormal = GenerateNormalMapFromHeightMap(barrelHeight, 2.0f);
    textures->barrel = LoadTextureFromImage(barrelImg);
    textures->barrelNormal = LoadTextureFromImage(barrelNormal);
    UnloadImage(barrelImg);
    UnloadImage(barrelNormal);

    Image crateImg = GenerateWoodTexture(256, 256);
    Image crateHeight = ExtractGrayscaleHeightMap(crateImg);
    Image crateNormal = GenerateNormalMapFromHeightMap(crateHeight, 2.0f);
    textures->crate = LoadTextureFromImage(crateImg);
    textures->crateNormal = LoadTextureFromImage(crateNormal);
    UnloadImage(crateImg);
    UnloadImage(crateNormal);

    textures->asphalt = LoadTextureFromImage(GenerateAsphaltTexture(512, 512));
    textures->brick = LoadTextureFromImage(GenerateBrickTexture(512, 512));
    textures->wood = LoadTextureFromImage(GenerateWoodTexture(512, 512));
    textures->fireGlow = LoadTextureFromImage(GenerateFireGlowTexture(256, 256));

    textures->camo = LoadTextureFromImage(GenerateCamoTexture(512, 512));
    textures->fence = LoadTextureFromImage(GenerateFenceTexture(512, 512));

    Image containerImg = GenerateContainerTexture(512, 512);
    Image containerHeight = ExtractGrayscaleHeightMap(containerImg);
    Image containerNormal = GenerateNormalMapFromHeightMap(containerHeight, 2.0f);
    textures->container = LoadTextureFromImage(containerImg);
    textures->containerNormal = LoadTextureFromImage(containerNormal);
    UnloadImage(containerImg);
    UnloadImage(containerNormal);

    textures->generated = true;
}

void TextureShutdown(ProceduralTextures *textures) {
    if (textures->generated) {
        UnloadTexture(textures->zombieSkin);
        UnloadTexture(textures->zombieSkinNormal);
        UnloadTexture(textures->zombieShirt);
        UnloadTexture(textures->zombiePants);
        UnloadTexture(textures->bloodDecal);
        UnloadTexture(textures->concrete);
        UnloadTexture(textures->concreteNormal);
        UnloadTexture(textures->defaultZombieHead);
        UnloadTexture(textures->metal);
        UnloadTexture(textures->metalNormal);
        UnloadTexture(textures->darkMetal);
        UnloadTexture(textures->grip);
        UnloadTexture(textures->barrel);
        UnloadTexture(textures->barrelNormal);
        UnloadTexture(textures->crate);
        UnloadTexture(textures->crateNormal);
        UnloadTexture(textures->asphalt);
        UnloadTexture(textures->brick);
        UnloadTexture(textures->wood);
        UnloadTexture(textures->fireGlow);
        UnloadTexture(textures->camo);
        UnloadTexture(textures->fence);
        UnloadTexture(textures->container);
        UnloadTexture(textures->containerNormal);
        textures->generated = false;
    }
}

Texture2D TextureCreateBloodSplatter(int width, int height) {
    Image img = GenerateBloodTexture(width, height);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D TextureCreateZombieSkin(int width, int height) {
    Image img = GenerateZombieSkin(width, height);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D TextureCreateNoiseTexture(int width, int height, float intensity) {
    Image img = GenImagePerlinNoise(width, height, 0, 0, intensity);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D TextureCreateUniformTexture(int width, int height) {
    Image img = GenerateUniformTexture(width, height);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D TextureCreateConcreteTexture(int width, int height) {
    Image img = GenerateConcreteTexture(width, height);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D TextureCreateMetalTexture(int width, int height) {
    Image img = GenerateMetalTexture(width, height);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}
