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
    for (int i = 0; i < 300; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 20 + rand() % 40;
        ImageDrawCircle(&img, x, y, r, (Color){ 120 + rand()%50, 0, 0, 160 + rand()%75 });
    }
    for (int i = 0; i < 50; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%40-20);
        int y2 = y1 + (rand()%40-20);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 100, 0, 0, 180 + rand()%60 });
    }
    return img;
}

static Image GenerateZombieSkin(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 70, 90, 55, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 35 + rand() % 70;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 20, shade - 10, 255 });
    }
    for (int i = 0; i < 20; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 2 + rand() % 5;
        ImageDrawCircle(&img, x, y, r, (Color){ 50, 60, 40, 200 });
    }
    return img;
}

static Image GenerateUniformTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 40, 45, 55, 255 });
    for (int i = 0; i < 400; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 30 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 5, shade + 15, 255 });
    }
    for (int i = 0; i < 30; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 3 + rand() % 6;
        ImageDrawCircle(&img, x, y, r, (Color){ 35, 40, 50, 180 });
    }
    return img;
}

static Image GenerateConcreteTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 70, 70, 75, 255 });
    for (int i = 0; i < 1000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 55 + rand() % 30;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 5, 255 });
    }
    for (int i = 0; i < 5; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%60-30);
        int y2 = y1 + (rand()%60-30);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 50, 50, 55, 100 });
    }
    return img;
}

static Image GenerateMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 90, 90, 95, 255 });
    for (int i = 0; i < 600; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 70 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade, shade + 3, 255 });
    }
    for (int i = 0; i < 10; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int w = 2 + rand() % 4;
        int h = 2 + rand() % 4;
        ImageDrawRectangle(&img, x, y, w, h, (Color){ 60, 60, 65, 120 });
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
        textures->generated = false;
    }
}
