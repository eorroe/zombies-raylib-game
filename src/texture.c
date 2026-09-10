#include "texture.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

static Image GenerateNoiseImage(int width, int height, float intensity) {
    Image img = GenImagePerlinNoise(width, height, 0, 0, intensity);
    return img;
}

static Image GenerateBloodTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 245, 240, 232, 255 });
    for (int i = 0; i < 600; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 20 + rand() % 80;
        ImageDrawCircle(&img, x, y, r, (Color){ 120, 20, 20, 200 + rand()%55 });
    }
    for (int i = 0; i < 300; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%200-100);
        int y2 = y1 + (rand()%200-100);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 80, 10, 10, 230 });
    }
    return img;
}

static Image GenerateZombieSkin(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 230, 210, 190, 255 });
    for (int i = 0; i < 3000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 180 + rand() % 60;
        ImageDrawPixel(&img, x, y, (Color){ shade + 20, shade - 10, shade - 30, 255 });
    }
    for (int i = 0; i < 150; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 4 + rand() % 12;
        ImageDrawCircle(&img, x, y, r, (Color){ 100, 50, 50, 220 });
    }
    for (int i = 0; i < 80; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        ImageDrawLine(&img, x1, y1, x1 + (rand()%20-10), y1 + (rand()%20-10), (Color){ 80, 40, 40, 180 });
    }
    return img;
}

static Image GenerateUniformTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 60, 90, 180, 255 });
    for (int i = 0; i < 1200; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 40 + rand() % 60;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 10, shade + 40, 255 });
    }
    for (int i = 0; i < 100; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 4 + rand() % 10;
        ImageDrawCircle(&img, x, y, r, (Color){ 30, 50, 150, 200 });
    }
    for (int i = 0; i < 60; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        ImageDrawLine(&img, x1, y1, x1 + (rand()%30-15), y1 + (rand()%30-15), (Color){ 40, 60, 160, 160 });
    }
    return img;
}

static Image GenerateConcreteTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 220, 215, 205, 255 });
    for (int i = 0; i < 3000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 190 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade - 5, shade - 10, 255 });
    }
    for (int i = 0; i < 30; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = x1 + (rand()%120-60);
        int y2 = y1 + (rand()%120-60);
        ImageDrawLine(&img, x1, y1, x2, y2, (Color){ 150, 145, 140, 130 });
    }
    return img;
}

static Image GenerateMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 160, 155, 150, 255 });
    for (int i = 0; i < 2000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 130 + rand() % 50;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade - 5, shade - 10, 255 });
    }
    for (int i = 0; i < 40; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int w = 3 + rand() % 6;
        int h = 3 + rand() % 6;
        ImageDrawRectangle(&img, x, y, w, h, (Color){ 110, 105, 100, 140 });
    }
    for (int i = 0; i < 25; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        ImageDrawLine(&img, x1, y1, x1 + (rand()%40-20), y1 + (rand()%40-20), (Color){ 90, 85, 80, 120 });
    }
    return img;
}

static Image GenerateDarkMetalTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 80, 78, 75, 255 });
    for (int i = 0; i < 1500; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 60 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade - 3, shade - 5, 255 });
    }
    for (int i = 0; i < 20; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        ImageDrawLine(&img, x1, y1, x1 + (rand()%30-15), y1 + (rand()%30-15), (Color){ 50, 48, 45, 100 });
    }
    return img;
}

static Image GenerateGripTexture(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 110, 85, 65, 255 });
    for (int i = 0; i < 1000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 80 + rand() % 40;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade - 10, shade - 20, 255 });
    }
    for (int i = 0; i < 60; i++) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        ImageDrawLine(&img, x1, y1, x1 + (rand()%20-10), y1 + (rand()%20-10), (Color){ 70, 50, 35, 160 });
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
