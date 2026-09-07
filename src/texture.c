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
    for (int i = 0; i < 200; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int r = 20 + rand() % 30;
        ImageDrawCircle(&img, x, y, r, (Color){ 120 + rand()%40, 0, 0, 180 + rand()%75 });
    }
    return img;
}

static Image GenerateZombieSkin(int width, int height) {
    Image img = GenImageColor(width, height, (Color){ 80, 100, 60, 255 });
    for (int i = 0; i < 500; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int shade = 40 + rand() % 60;
        ImageDrawPixel(&img, x, y, (Color){ shade, shade + 20, shade - 10, 255 });
    }
    return img;
}

void TextureGenerate(ProceduralTextures *textures) {
    textures->zombieSkin = LoadTextureFromImage(GenerateZombieSkin(256, 256));
    textures->zombieShirt = LoadTextureFromImage(GenImageGradientLinear(128, 128, 0, (Color){ 40, 50, 40, 255 }, (Color){ 20, 25, 20, 255 }));
    textures->zombiePants = LoadTextureFromImage(GenImageGradientLinear(128, 128, 0, (Color){ 30, 30, 40, 255 }, (Color){ 15, 15, 20, 255 }));
    textures->bloodDecal = LoadTextureFromImage(GenerateBloodTexture(256, 256));
    textures->concrete = LoadTextureFromImage(GenImagePerlinNoise(256, 256, 0, 0, 0.5f));
    textures->defaultZombieHead = LoadTextureFromImage(GenerateZombieSkin(128, 128));
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

void TextureShutdown(ProceduralTextures *textures) {
    if (textures->generated) {
        UnloadTexture(textures->zombieSkin);
        UnloadTexture(textures->zombieShirt);
        UnloadTexture(textures->zombiePants);
        UnloadTexture(textures->bloodDecal);
        UnloadTexture(textures->concrete);
        UnloadTexture(textures->defaultZombieHead);
        textures->generated = false;
    }
}
