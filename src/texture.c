#include "texture.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

static Image GenerateNoiseImage(int width, int height, float intensity) {
    Image img = GenImagePerlinNoise(width, height, 0, 0, intensity);
    return img;
}

static Image GenerateBloodTexture(int width, int height) {
    Image img = GenImageColor(width, height, BLANK);
    for (int i = 0; i < 400; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 30 + rand() % 50;
        ImageDrawCircle(&img, x, y, r, (Color){ 180 + rand()%50, 0, 0, 200 + rand()%55 });
    }
    for (int i = 0; i < 80; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%60-30);
        int y2 = y1 + (rand()%60-30);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 160, 0, 0, 220 + rand()%35 });
    }
    return img;
}

static Image GenerateZombieSkin(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 140, 170, 110, 255 });
    for (int i = 0; i < 1200; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 100 + rand() % 100;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 40, shade - 10, 255 });
    }
    for (int i = 0; i < 40; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 3 + rand() % 8;
        ImageDrawCircle(&img, x, y, r, (Color){ 90, 120, 70, 220 });
    }
    return img;
}

static Image GenerateUniformTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 90, 95, 110, 255 });
    for (int i = 0; i < 600; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 70 + rand() % 60;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 8, shade + 20, 255 });
    }
    for (int i = 0; i < 50; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 4 + rand() % 8;
        ImageDrawCircle(&img, x, y, r, (Color){ 70, 75, 90, 200 });
    }
    return img;
}

static Image GenerateConcreteTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 120, 120, 125, 255 });
    for (int i = 0; i < 1500; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 100 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 5, 255 });
    }
    for (int i = 0; i < 8; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%80-40);
        int y2 = y1 + (rand()%80-40);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 90, 90, 95, 120 });
    }
    return img;
}

static Image GenerateMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 150, 150, 155, 255 });
    for (int i = 0; i < 1000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 120 + rand() % 50;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 3, 255 });
    }
    for (int i = 0; i < 15; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int w = 3 + rand() % 6;
        int h = 3 + rand() % 6;
        ImageDrawRectangle(&img, x, y, w, h, (Color){ 100, 100, 105, 150 });
    }
    return img;
}

static Image GenerateDarkMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 80, 80, 85, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 60 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 2, 255 });
    }
    return img;
}

static Image GenerateGripTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 50, 40, 35, 255 });
    for (int i = 0; i < 600; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 35 + rand() % 30;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade - 5, shade - 10, 255 });
    }
    return img;
}

void TextureGenerate(ProceduralTextures *textures) {
    textures->zombieSkin = LoadTextureFromImage(GenerateZombieSkin(256, 256));
    textures->zombieShirt = LoadTextureFromImage(GenerateUniformTexture(256, 256));
    textures->zombiePants = LoadTextureFromImage(GenerateUniformTexture(256, 256));
    textures->bloodDecal = LoadTextureFromImage(GenerateBloodTexture(256, 256));
    textures->concrete = LoadTextureFromImage(GenerateConcreteTexture(256, 256));
    textures->defaultZombieHead = LoadTextureFromImage(GenerateZombieSkin(128, 128));
    textures->metal = LoadTextureFromImage(GenerateMetalTexture(256, 256));
    textures->darkMetal = LoadTextureFromImage(GenerateDarkMetalTexture(256, 256));
    textures->grip = LoadTextureFromImage(GenerateGripTexture(256, 256));
    textures->generated = true;
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
    Image img = GenerateNoiseImage(width, height, intensity);
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

void TextureShutdown(ProceduralTextures *textures) {
    if (textures->generated) {
        UnloadTexture(textures->zombieSkin);
        UnloadTexture(textures->zombieShirt);
        UnloadTexture(textures->zombiePants);
        UnloadTexture(textures->bloodDecal);
        UnloadTexture(textures->concrete);
        UnloadTexture(textures->defaultZombieHead);
        UnloadTexture(textures->metal);
        UnloadTexture(textures->darkMetal);
        UnloadTexture(textures->grip);
        textures->generated = false;
    }
}
