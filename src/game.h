#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "zombie.h"
#include "player.h"
#include "weapon.h"
#include "camera.h"
#include "ui.h"
#include "texture.h"
#include "image_upload.h"
#include "shader.h"
#include "particle.h"
#include "audio.h"
#include "input.h"
#include "debug.h"

typedef struct Game {
    GameState state;
    GameMode mode;
    ZombieMode zombieMode;
    int score;
    int round;
    int zombiesRemaining;
    float gameTime;
    bool scopeActive;
    
    Texture2D zombieHeadTextures[16];
    int zombieHeadTextureCount;
    ShaderManager shaders;
    RenderTexture2D sceneTarget;
    RenderTexture2D postProcessTarget;
    
    Player player;
    Weapon weapon;
    GameCamera camera;
    Zombie zombies[64];
    int zombieCount;
    Particle particles[256];
    int particleCount;
    Vector3 bloodDecals[128];
    int bloodDecalCount;
    Vector3 muzzleFlashPos;
    float muzzleFlashTimer;
    AudioManager audio;
    ProceduralTextures textures;
    ImageUpload imageUpload;
    DebugState debug;
    
    MenuState menu;
} Game;

void GameInit(Game *game, int screenWidth, int screenHeight);
void GameUpdate(Game *game, float dt);
void GameRender(Game *game);
void GameShutdown(Game *game);

#endif
