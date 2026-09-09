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
    ZombieInit(&game->zombies[idx], pos, type, texIdx, game->textures.zombieSkin, game->textures.zombieShirt, game->textures.zombiePants);
}

static void SpawnWave(Game *game) {
    game->round++;
    game->zombiesRemaining = 5 + game->round * 3;
    for (int i = 0; i < game->zombiesRemaining; i++) {
        float angle = (float)i / game->zombiesRemaining * 2.0f * PI;
        float radius = 6.0f + rand() % 8;
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
    game->bloodDecalCount = 0;
    game->zombieHeadTextureCount = 0;
    game->muzzleFlashTimer = 0.0f;
    game->muzzleFlashPos = (Vector3){ 0 };
    
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

static bool PointInAABB(Vector3 p, Vector3 center, Vector3 size) {
    Vector3 half = Vector3Scale(size, 0.5f);
    Vector3 min = Vector3Subtract(center, half);
    Vector3 max = Vector3Add(center, half);
    return p.x >= min.x && p.x <= max.x &&
           p.z >= min.z && p.z <= max.z;
}

static void GameApplyCollisions(Game *game) {
    Vector3 p = game->player.position;
    float radius = PLAYER_RADIUS;
    
    for (int i = 0; i < game->zombieCount; i++) {
        if (!ZombieIsAlive(&game->zombies[i])) continue;
        float dist = Vector3Length(Vector3Subtract(p, game->zombies[i].position));
        float minDist = radius + 0.4f;
        if (dist < minDist && dist > 0.001f) {
            Vector3 push = Vector3Normalize(Vector3Subtract(p, game->zombies[i].position));
            game->player.position = Vector3Add(game->player.position, Vector3Scale(push, minDist - dist));
        }
    }
    
    for (int i = 0; i < 10; i++) {
        float x = -18.0f + i * 4.0f;
        Vector3 wallCenter = { x, 0.75f, -10.0f };
        Vector3 wallSize = { 4.0f, 1.5f, 0.6f };
        if (PointInAABB(p, wallCenter, wallSize)) {
            float dx = p.x - wallCenter.x;
            float dz = p.z - wallCenter.z;
            float halfX = wallSize.x * 0.5f + radius;
            float halfZ = wallSize.z * 0.5f + radius;
            if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                game->player.position.x = wallCenter.x + (dx > 0 ? halfX : -halfX);
            } else {
                game->player.position.z = wallCenter.z + (dz > 0 ? halfZ : -halfZ);
            }
        }
    }
    
    for (int i = 0; i < 12; i++) {
        float x = -12.0f + i * 4.0f;
        float z = 10.0f;
        float h = 1.5f + (i % 3) * 1.0f;
        Vector3 bldPos = { x, h * 0.5f, z };
        Vector3 bldSize = { 3.5f, h, 3.5f };
        if (PointInAABB(p, bldPos, bldSize)) {
            float dx = p.x - bldPos.x;
            float dz = p.z - bldPos.z;
            float halfX = bldSize.x * 0.5f + radius;
            float halfZ = bldSize.z * 0.5f + radius;
            if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                game->player.position.x = bldPos.x + (dx > 0 ? halfX : -halfX);
            } else {
                game->player.position.z = bldPos.z + (dz > 0 ? halfZ : -halfZ);
            }
        }
    }
    
    for (int i = 0; i < 16; i++) {
        float angle = i * PI * 0.25f;
        float rad = 5.0f + (i % 3) * 3.0f;
        Vector3 cratePos = { cosf(angle) * rad, 0.3f, sinf(angle) * rad };
        if (PointInAABB(p, cratePos, (Vector3){ 1.0f, 1.0f, 1.0f })) {
            float dx = p.x - cratePos.x;
            float dz = p.z - cratePos.z;
            float dist2 = sqrtf(dx*dx + dz*dz);
            if (dist2 > 0.001f) {
                float pushDist = 1.0f * 0.5f + radius;
                game->player.position.x = cratePos.x + (dx / dist2) * pushDist;
                game->player.position.z = cratePos.z + (dz / dist2) * pushDist;
            }
        }
    }
    
    for (int i = 0; i < 10; i++) {
        float angle = i * PI * 0.5f + 0.3f;
        float rad = 3.0f + (i % 2) * 3.0f;
        Vector3 barrelPos = { cosf(angle) * rad, 0.4f, sinf(angle) * rad };
        if (PointInAABB(p, barrelPos, (Vector3){ 0.6f, 1.2f, 0.6f })) {
            float dx = p.x - barrelPos.x;
            float dz = p.z - barrelPos.z;
            float dist2 = sqrtf(dx*dx + dz*dz);
            if (dist2 > 0.001f) {
                float pushDist = 0.6f * 0.5f + radius;
                game->player.position.x = barrelPos.x + (dx / dist2) * pushDist;
                game->player.position.z = barrelPos.z + (dz / dist2) * pushDist;
            }
        }
    }
}

static void GameApplyZombieCollisions(Game *game) {
    float radius = 0.4f;
    for (int i = 0; i < game->zombieCount; i++) {
        if (!ZombieIsAlive(&game->zombies[i])) continue;
        Vector3 p = game->zombies[i].position;
        for (int w = 0; w < 10; w++) {
            float x = -18.0f + w * 4.0f;
            Vector3 wallCenter = { x, 0.75f, -10.0f };
            Vector3 wallSize = { 4.0f, 1.5f, 0.6f };
            if (PointInAABB(p, wallCenter, wallSize)) {
                float dx = p.x - wallCenter.x;
                float dz = p.z - wallCenter.z;
                float halfX = wallSize.x * 0.5f + radius;
                float halfZ = wallSize.z * 0.5f + radius;
                if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                    game->zombies[i].position.x = wallCenter.x + (dx > 0 ? halfX : -halfX);
                } else {
                    game->zombies[i].position.z = wallCenter.z + (dz > 0 ? halfZ : -halfZ);
                }
            }
        }
        for (int b = 0; b < 12; b++) {
            float x = -12.0f + b * 4.0f;
            float z = 10.0f;
            float h = 1.5f + (b % 3) * 1.0f;
            Vector3 bldPos = { x, h * 0.5f, z };
            Vector3 bldSize = { 3.5f, h, 3.5f };
            if (PointInAABB(p, bldPos, bldSize)) {
                float dx = p.x - bldPos.x;
                float dz = p.z - bldPos.z;
                float halfX = bldSize.x * 0.5f + radius;
                float halfZ = bldSize.z * 0.5f + radius;
                if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                    game->zombies[i].position.x = bldPos.x + (dx > 0 ? halfX : -halfX);
                } else {
                    game->zombies[i].position.z = bldPos.z + (dz > 0 ? halfZ : -halfZ);
                }
            }
        }
        
        for (int c = 0; c < 16; c++) {
            float angle = c * PI * 0.25f;
            float rad = 5.0f + (c % 3) * 3.0f;
            Vector3 cratePos = { cosf(angle) * rad, 0.3f, sinf(angle) * rad };
            if (PointInAABB(p, cratePos, (Vector3){ 1.0f, 1.0f, 1.0f })) {
                float dx = p.x - cratePos.x;
                float dz = p.z - cratePos.z;
                float dist2 = sqrtf(dx*dx + dz*dz);
                if (dist2 > 0.001f) {
                    float pushDist = 1.0f * 0.5f + radius;
                    game->zombies[i].position.x = cratePos.x + (dx / dist2) * pushDist;
                    game->zombies[i].position.z = cratePos.z + (dz / dist2) * pushDist;
                }
            }
        }
        
        for (int br = 0; br < 10; br++) {
            float angle = br * PI * 0.5f + 0.3f;
            float rad = 3.0f + (br % 2) * 3.0f;
            Vector3 barrelPos = { cosf(angle) * rad, 0.4f, sinf(angle) * rad };
            if (PointInAABB(p, barrelPos, (Vector3){ 0.6f, 1.2f, 0.6f })) {
                float dx = p.x - barrelPos.x;
                float dz = p.z - barrelPos.z;
                float dist2 = sqrtf(dx*dx + dz*dz);
                if (dist2 > 0.001f) {
                    float pushDist = 0.6f * 0.5f + radius;
                    game->zombies[i].position.x = barrelPos.x + (dx / dist2) * pushDist;
                    game->zombies[i].position.z = barrelPos.z + (dz / dist2) * pushDist;
                }
            }
        }
    }
}

void GameUpdate(Game *game, float dt, InputState *input) {
    DebugUpdate(&game->debug, dt);
    if (IsKeyPressed(KEY_F1)) DebugToggle(&game->debug);
    if (IsKeyPressed(KEY_F2)) DebugClear(&game->debug);
    
    if (game->state != GAME_STATE_PLAYING) return;
    
    game->gameTime += dt;
    PlayerUpdate(&game->player, input, dt);
    CameraUpdate(&game->camera, &game->player, dt);
    WeaponUpdate(&game->weapon, game->player.position, input, dt);
    
    GameApplyCollisions(game);
    
    DebugLogf(&game->debug, DEBUG_INFO, "Player health: %.1f", game->player.health);
    DebugLogf(&game->debug, DEBUG_INFO, "Zombies alive: %d", game->zombieCount);
    
    bool invincible = game->gameTime < 8.0f;
    
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieUpdate(&game->zombies[i], game->player.position, dt);
        if (!invincible && ZombieIsAlive(&game->zombies[i]) && game->zombies[i].attackCooldown <= 0) {
            float dist = Vector3Length(Vector3Subtract(game->zombies[i].position, game->player.position));
            if (dist < 2.0f) {
                PlayerTakeDamage(&game->player, 5.0f);
                game->zombies[i].attackCooldown = 2.0f;
                AudioPlayPlayerHit(&game->audio);
            }
        }
    }
    
    GameApplyZombieCollisions(game);
    
    if (game->player.health <= 0) {
        game->state = GAME_STATE_GAMEOVER;
    }
    
    if (input->mouseLeftReleased && WeaponCanShoot(&game->weapon)) {
        WeaponShoot(&game->weapon);
        AudioPlayGunshot(&game->audio);
        
        RayHitInfo hit = WeaponRaycast(&game->weapon, game->camera.camera, game->zombies, game->zombieCount);
        if (hit.hit) {
            ZombieTakeDamage(&game->zombies[hit.zombieIndex], WEAPON_DAMAGE);
            if (!ZombieIsAlive(&game->zombies[hit.zombieIndex])) {
                game->score += 100;
                DebugLogf(&game->debug, DEBUG_INFO, "Score: %d", game->score);
                AudioPlayZombieGrowl(&game->audio);
                Vector3 deathPos = game->zombies[hit.zombieIndex].position;
                for (int p = 0; p < 30; p++) {
                    if (game->particleCount < 256) {
                        Vector3 bloodVel = {
                            (rand()%100-50)/25.0f,
                            (rand()%100)/15.0f,
                            (rand()%100-50)/25.0f
                        };
                        ParticleSpawn(&game->particles[game->particleCount++], deathPos,
                            bloodVel, 2.5f, PARTICLE_BLOOD, 0.12f + rand()%100/800.0f, (Color){ 200 + rand()%55, 0, 0, 255 });
                    }
                }
                if (game->bloodDecalCount < 128) {
                    game->bloodDecals[game->bloodDecalCount++] = deathPos;
                }
                for (int p = 0; p < 3; p++) {
                    if (game->particleCount < 256) {
                        Vector3 splashPos = deathPos;
                        splashPos.y = 0.05f;
                        ParticleSpawn(&game->particles[game->particleCount++], splashPos,
                            (Vector3){ (rand()%100-50)/80.0f, 0.05f, (rand()%100-50)/80.0f },
                            3.0f, PARTICLE_BLOOD, 0.25f + rand()%100/500.0f, (Color){ 180, 0, 0, 220 });
                    }
                }
            }
            for (int p = 0; p < 5; p++) {
                if (game->particleCount < 256) {
                    ParticleSpawn(&game->particles[game->particleCount++], hit.point,
                        (Vector3){ (rand()%100-50)/50.0f, (rand()%100-50)/50.0f, (rand()%100-50)/50.0f },
                        1.0f, PARTICLE_BLOOD, 0.05f, RED);
                }
            }
        }
        
        game->muzzleFlashPos = game->weapon.position;
        game->muzzleFlashTimer = 0.05f;
    }
    
    
    if (IsKeyPressed(KEY_R)) WeaponReload(&game->weapon);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) game->scopeActive = !game->scopeActive;
    if (game->scopeActive) CameraApplyScope(&game->camera, true);
    else CameraApplyScope(&game->camera, false);
    
    ParticleSystemUpdate(game->particles, game->particleCount, dt);
    
    if (game->muzzleFlashTimer > 0) game->muzzleFlashTimer -= dt;
    
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
    RendererDrawBloodDecals(game);
    RendererDrawZombies(game, game->shaders.pbr);
    WeaponRender(&game->weapon, cam);
    RendererDrawParticles(game->particles, game->particleCount);
    RendererEnd(game);
    RendererDrawHUD(game);
    
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
    int screenshotFrame = 120;
    
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F1)) DebugToggle(&game.debug);
        if (IsKeyPressed(KEY_F2)) DebugClear(&game.debug);
        InputUpdate(&input, game.menu.active);
        UIUpdate(&game.menu, &input, &game);
        
        static bool cursorEnabled = true;
        if (game.state == GAME_STATE_PLAYING && !game.menu.active) {
            if (cursorEnabled) {
                DisableCursor();
                cursorEnabled = false;
            }
        } else {
            if (!cursorEnabled) {
                EnableCursor();
                cursorEnabled = true;
            }
        }
        
        if (autoStart && frameCount == autoStartFrame && game.state == GAME_STATE_MENU) {
            game.state = GAME_STATE_PLAYING;
            GameInit(&game, screenWidth, screenHeight);
        }
        
        if (game.state == GAME_STATE_PLAYING && !game.menu.active) {
            GameUpdate(&game, GetFrameTime(), &input);
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (game.state == GAME_STATE_MENU || game.menu.active) {
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
        
        if (screenshotPath && frameCount == screenshotFrame) {
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
