#ifndef TEXTURE_H
#define TEXTURE_H

#include "raylib.h"

typedef struct {
    Texture2D zombieSkin;
    Texture2D zombieSkinNormal;
    Texture2D zombieShirt;
    Texture2D zombiePants;
    Texture2D bloodDecal;
    Texture2D concrete;
    Texture2D concreteNormal;
    Texture2D defaultZombieHead;
    Texture2D metal;
    Texture2D metalNormal;
    Texture2D darkMetal;
    Texture2D grip;
    Texture2D asphalt;
    Texture2D brick;
    Texture2D wood;
    Texture2D fireGlow;
    Texture2D barrel;
    Texture2D barrelNormal;
    Texture2D crate;
    Texture2D crateNormal;
    bool generated;
} ProceduralTextures;

void TextureGenerate(ProceduralTextures *textures);
void TextureShutdown(ProceduralTextures *textures);
Texture2D TextureCreateBloodSplatter(int width, int height);
Texture2D TextureCreateZombieSkin(int width, int height);
Texture2D TextureCreateUniformTexture(int width, int height);
Texture2D TextureCreateConcreteTexture(int width, int height);
Texture2D TextureCreateMetalTexture(int width, int height);
Texture2D TextureCreateNoiseTexture(int width, int height, float intensity);

#endif
