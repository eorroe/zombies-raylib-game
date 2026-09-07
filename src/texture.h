#ifndef TEXTURE_H
#define TEXTURE_H

#include "raylib.h"

typedef struct {
    Texture2D zombieSkin;
    Texture2D zombieShirt;
    Texture2D zombiePants;
    Texture2D bloodDecal;
    Texture2D concrete;
    Texture2D defaultZombieHead;
    bool generated;
} ProceduralTextures;

void TextureGenerate(ProceduralTextures *textures);
void TextureShutdown(ProceduralTextures *textures);
Texture2D TextureCreateBloodSplatter(int width, int height);
Texture2D TextureCreateZombieSkin(int width, int height);
Texture2D TextureCreateNoiseTexture(int width, int height, float intensity);

#endif
