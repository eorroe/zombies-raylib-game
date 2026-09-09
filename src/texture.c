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
    for (int i = 0; i < 600; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 40 + rand() % 60;
        ImageDrawCircle(&img, x, y, r, (Color){ 220 + rand()%35, 10, 10, 220 + rand()%35 });
    }
    for (int i = 0; i < 120; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%80-40);
        int y2 = y1 + (rand()%80-40);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 200, 10, 10, 240 + rand()%15 });
    }
    return img;
}

static Image GenerateZombieSkin(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 180, 220, 140, 255 });
    for (int i = 0; i < 1500; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 140 + rand() % 100;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 50, shade - 5, 255 });
    }
    for (int i = 0; i < 60; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 4 + rand() % 10;
        ImageDrawCircle(&img, x, y, r, (Color){ 130, 180, 110, 240 });
    }
    return img;
}

static Image GenerateUniformTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 130, 135, 160, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 110 + rand() % 70;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 10, shade + 25, 255 });
    }
    for (int i = 0; i < 60; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 5 + rand() % 10;
        ImageDrawCircle(&img, x, y, r, (Color){ 110, 115, 140, 220 });
    }
    return img;
}

static Image GenerateConcreteTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 160, 160, 165, 255 });
    for (int i = 0; i < 2000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 140 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 5, 255 });
    }
    for (int i = 0; i < 12; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%100-50);
        int y2 = y1 + (rand()%100-50);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 130, 130, 135, 140 });
    }
    return img;
}

static Image GenerateMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 180, 180, 185, 255 });
    for (int i = 0; i < 1200; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 160 + rand() % 50;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 3, 255 });
    }
    for (int i = 0; i < 20; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int w = 4 + rand() % 8;
        int h = 4 + rand() % 8;
        ImageDrawRectangle(&img, x, y, w, h, (Color){ 140, 140, 145, 160 });
    }
    return img;
}

static Image GenerateDarkMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 100, 100, 105, 255 });
    for (int i = 0; i < 1000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 80 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 2, 255 });
    }
    return img;
}

static Image GenerateGripTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 70, 55, 45, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 50 + rand() % 35;
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
