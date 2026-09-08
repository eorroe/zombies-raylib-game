#include "game.h"
#include "debug.h"
#include "input.h"
#include "renderer.h"
#include "audio.h"
#include "raymath.h"
#include <stdlib.h>
#include <time.h>

static void SpawnZombie(Game *game, Vector3 pos, ZombieType type) {
    if (game->zombieCount >= MAX_ZOMBIES) return;
    int idx = game->zombieCount++;
    int texIdx = 0;
    if (type == ZOMBIE_TYPE_IMAGE_HEAD && game->zombieHeadTextureCount > 0) {
        texIdx = rand() % game->zombieHeadTextureCount;
        DebugLogf(&game->debug, DEBUG_INFO, "Spawning image-head zombie with texIdx=%d", texIdx);
    }
    ZombieInit(&game->zombies[idx], pos, type, texIdx);
}

static void SpawnWave(Game *game) {
    game->round++;
    game->zombiesRemaining = 5 + game->round * 3;
    for (int i = 0; i < game->zombiesRemaining; i++) {
        float angle = (float)i / game->zombiesRemaining * 2.0f * PI;
        float radius = 15.0f + rand() % 10;  // was 20.0f + rand() % 10;
        Vector3 pos = {
            cosf(angle) * radius,
            0,
            sinf(angle) * radius
        };
        ZombieType type = ZOMBIE_TYPE_DEFAULT;
        if (game->zombieMode == ZOMBIE_MODE_ALL_IMAGES && game->zombieHeadTextureCount > 0) {
            type = ZOMBIE_TYPE_IMAGE_HEAD;
        } else if (game->zombieMode == ZOMBIE_MODE_MIXED && rand() % 10 == 0 && game->zombieHeadTextureCount > 0) {
            type = ZOMBIE_TYPE_IMAGE_HEAD;
        }
        SpawnZombie(game, pos, type);
    }
}

void GameInit(Game *game, int screenWidth, int screenHeight) {
    DebugInit(&game->debug, 10.0f);
    DebugLog(&game->debug, "Game started", DEBUG_SUCCESS);
    game->state = GAME_STATE_PLAYING;
    game->score = 0;
    game->round = 0;
    game->gameTime = 0.0f;
    game->scopeActive = false;
    game->zombieCount = 0;
    game->particleCount = 0;
    game->zombieHeadTextureCount = 0;
    
    PlayerInit(&game->player, (Vector3){ 0, 0, 0 });
    WeaponInit(&game->weapon);
    CameraInit(&game->camera, &game->player);
    
    AudioInit(&game->audio);
    TextureGenerate(&game->textures);
    RendererInit(game, screenWidth, screenHeight);
    game->mode = game->menu.mode;
    game->zombieMode = game->menu.zombieMode;
    
    if (game->menu.uploadedImageCount > 0) {
        game->zombieHeadTextureCount = game->menu.uploadedImageCount;
        for (int i = 0; i < game->zombieHeadTextureCount && i < 16; i++) {
            game->zombieHeadTextures[i] = game->menu.uploadedImages[i];
        }
        DebugLogf(&game->debug, DEBUG_SUCCESS, "Loaded %d zombie head textures", game->zombieHeadTextureCount);
    } else {
        DebugLog(&game->debug, "No zombie head textures uploaded", DEBUG_WARN);
    }
    
    SpawnWave(game);
}

void GameUpdate(Game *game, float dt) {
    DebugUpdate(&game->debug, dt);
    if (IsKeyPressed(KEY_F1)) DebugToggle(&game->debug);
    if (IsKeyPressed(KEY_F2)) DebugClear(&game->debug);
    
    if (game->state != GAME_STATE_PLAYING) return;
    
    game->gameTime += dt;
    PlayerUpdate(&game->player, CameraGetCamera(&game->camera), dt);
    CameraUpdate(&game->camera, &game->player, dt);
    WeaponUpdate(&game->weapon, game->camera.camera, dt);
    
    DebugLogf(&game->debug, DEBUG_INFO, "Player health: %.1f", game->player.health);
    DebugLogf(&game->debug, DEBUG_INFO, "Zombies alive: %d", game->zombieCount);
    
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieUpdate(&game->zombies[i], game->player.position, dt);
        if (ZombieIsAlive(&game->zombies[i]) && game->zombies[i].attackCooldown <= 0) {
            float dist = Vector3Length(Vector3Subtract(game->zombies[i].position, game->player.position));
            if (dist < 2.0f) {
                PlayerTakeDamage(&game->player, 10.0f);
                game->zombies[i].attackCooldown = 1.0f;
                AudioPlayPlayerHit(&game->audio);
            }
        }
    }
    
    if (game->player.health <= 0) {
        game->state = GAME_STATE_GAMEOVER;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        WeaponShoot(&game->weapon);
        AudioPlayGunshot(&game->audio);
        
        RayHitInfo hit = WeaponRaycast(&game->weapon, game->camera.camera, game->zombies, game->zombieCount);
        if (hit.hit) {
            ZombieTakeDamage(&game->zombies[hit.zombieIndex], WEAPON_DAMAGE);
            if (!ZombieIsAlive(&game->zombies[hit.zombieIndex])) {
                game->score += 100;
                DebugLogf(&game->debug, DEBUG_INFO, "Score: %d", game->score);
                AudioPlayZombieGrowl(&game->audio);
            }
            for (int p = 0; p < 5; p++) {
                if (game->particleCount < 256) {
                    ParticleSpawn(&game->particles[game->particleCount++], hit.point,
                        (Vector3){ (rand()%100-50)/50.0f, (rand()%100-50)/50.0f, (rand()%100-50)/50.0f },
                        1.0f, PARTICLE_BLOOD, 0.05f, RED);
                }
            }
        }
    }
    
    if (IsKeyPressed(KEY_R)) WeaponReload(&game->weapon);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) game->scopeActive = !game->scopeActive;
    if (game->scopeActive) CameraApplyScope(&game->camera, true);
    else CameraApplyScope(&game->camera, false);
    
    ParticleSystemUpdate(game->particles, game->particleCount, dt);
    
    bool allDead = true;
    for (int i = 0; i < game->zombieCount; i++) {
        if (ZombieIsAlive(&game->zombies[i])) { allDead = false; break; }
    }
    if (allDead) {
        SpawnWave(game);
    }
}

void GameRender(Game *game) {
    Camera3D cam = CameraGetCamera(&game->camera);
    RendererBegin(game, cam);
    RendererDrawScene(game);
    RendererDrawZombies(game, game->shaders.pbr);
    RendererDrawPlayer(&game->player, game->shaders.pbr);
    RendererDrawParticles(game->particles, game->particleCount);
    RendererEnd(game);
    
    if (game->debug.enabled) DebugRender(&game->debug, 1280, 720);
}

void GameShutdown(Game *game) {
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieShutdown(&game->zombies[i]);
    }
    PlayerShutdown(&game->player);
    WeaponShutdown(&game->weapon);
    AudioShutdown(&game->audio);
    TextureShutdown(&game->textures);
    DebugShutdown(&game->debug);
    RendererShutdown(game);
}

int main(void) {
    srand(time(NULL));
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    InitWindow(screenWidth, screenHeight, "Zombie Shooter - AAA Edition");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    EnableCursor();
    
    Game game = { 0 };
    UIInit(&game.menu, screenWidth, screenHeight);
    DebugInit(&game.debug, 10.0f);
    game.state = GAME_STATE_MENU;
    
    InputState input;
    int frameCount = 0;
    const char *screenshotPath = getenv("ZOMBIE_SHOT");
    int autoQuitMs = -1;
    const char *autoQuitStr = getenv("ZOMBIE_AUTO_QUIT_MS");
    if (autoQuitStr) autoQuitMs = atoi(autoQuitStr);
    double startTime = GetTime();
    bool autoStart = getenv("ZOMBIE_AUTO_START") != NULL;
    int autoStartFrame = 30;
    
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F1)) DebugToggle(&game.debug);
        if (IsKeyPressed(KEY_F2)) DebugClear(&game.debug);
        InputUpdate(&input, game.menu.active);
        UIUpdate(&game.menu, &input, &game);
        
        if (autoStart && frameCount == autoStartFrame && game.state == GAME_STATE_MENU) {
            game.state = GAME_STATE_PLAYING;
            GameInit(&game, screenWidth, screenHeight);
        }
        
        if (game.state == GAME_STATE_PLAYING) {
            GameUpdate(&game, GetFrameTime());
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (game.state == GAME_STATE_MENU) {
            UIRender(&game.menu);
            if (game.debug.enabled) DebugRender(&game.debug, screenWidth, screenHeight);
        } else if (game.state == GAME_STATE_PLAYING) {
            GameRender(&game);
        } else if (game.state == GAME_STATE_GAMEOVER) {
            DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 20, 40, RED);
            DrawText(TextFormat("Final Score: %d", game.score), screenWidth / 2 - MeasureText(TextFormat("Final Score: %d", game.score), 20) / 2, screenHeight / 2 + 30, 20, WHITE);
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                GameShutdown(&game);
                UIInit(&game.menu, screenWidth, screenHeight);
                game.state = GAME_STATE_MENU;
            }
            if (game.debug.enabled) DebugRender(&game.debug, screenWidth, screenHeight);
        }
        
        EndDrawing();
        
        if (screenshotPath && frameCount == 60) {
            TakeScreenshot(screenshotPath);
        }
        frameCount++;
        
        if (autoQuitMs > 0 && (GetTime() - startTime) * 1000.0 > autoQuitMs) break;
    }
    
    if (game.state == GAME_STATE_PLAYING) {
        GameShutdown(&game);
    }
    
    CloseWindow();
    return 0;
}
