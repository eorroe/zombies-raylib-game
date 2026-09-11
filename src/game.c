#include "game.h"
#include "debug.h"
#include "input.h"
#include "renderer.h"
#include "audio.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <fcntl.h>
#else
#include <dirent.h>
#endif

static int FindNextIteration(const char *dir) {
#if defined(_WIN32) || defined(_WIN64)
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s/iteration_*.png", dir);
    intptr_t handle = _findfirst(pattern, NULL);
    if (handle == -1) return 1;
    int maxNum = 0;
    struct _finddata_t info;
    do {
        int n;
        if (sscanf(info.name, "iteration_%d.png", &n) == 1 && n > maxNum) {
            maxNum = n;
        }
    } while (_findnext(handle, &info) == 0);
    _findclose(handle);
    return maxNum + 1;
#else
    DIR *d = opendir(dir);
    if (!d) return 1;
    int maxNum = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        int n;
        if (sscanf(ent->d_name, "iteration_%d.png", &n) == 1 && n > maxNum) {
            maxNum = n;
        }
    }
    closedir(d);
    return maxNum + 1;
#endif
}

static void SpawnZombie(Game *game, Vector3 pos, ZombieType type, int texIdx) {
    if (game->zombieCount >= MAX_ZOMBIES) return;
    int idx = game->zombieCount++;
    if (type == ZOMBIE_TYPE_IMAGE_HEAD && game->zombieHeadTextureCount > 0) {
        texIdx = texIdx % game->zombieHeadTextureCount;
        DebugLogf(&game->debug, DEBUG_INFO, "Spawning image-head zombie with texIdx=%d", texIdx);
    }
    ZombieInit(&game->zombies[idx], pos, type, texIdx, game->textures.zombieSkin, game->textures.zombieSkinNormal, game->textures.zombieShirt, game->textures.zombiePants, game->textures.zombieBone, game->shaders.pbr);
    game->zombies[idx].speed = ZOMBIE_SPEED_BASE;
    if (type == ZOMBIE_TYPE_IMAGE_HEAD && game->zombieMode == ZOMBIE_MODE_MIXED) {
        game->zombies[idx].speed = ZOMBIE_SPEED_BASE * 2.0f;
    }
}

static Vector3 GetZombieSpawnPoint(void) {
    float angle = (float)rand() / RAND_MAX * PI;
    float radius = 5.0f + rand() % 15;
    Vector3 pos = {
        (float)(rand() % 20 - 10),
        0,
        (float)(5 + rand() % 20)
    };
    return pos;
}

static void SpawnImageZombie(Game *game, int texIdx) {
    Vector3 pos = GetZombieSpawnPoint();
    SpawnZombie(game, pos, ZOMBIE_TYPE_IMAGE_HEAD, texIdx);
}

static void SpawnWave(Game *game) {
    game->round++;
    game->zombiesRemaining = 8 + game->round * 4;
    game->nonImageDeathsSinceLastImage = 0;
    bool usedImages[16] = { false };
    int imageCount = 0;
    if (game->zombieMode == ZOMBIE_MODE_ALL_IMAGES && game->zombieHeadTextureCount > 0) {
        imageCount = game->zombieHeadTextureCount > 0 ? 1 : 0;
    } else if (game->zombieMode == ZOMBIE_MODE_MIXED && game->zombieHeadTextureCount > 0) {
        imageCount = 1;
        usedImages[rand() % game->zombieHeadTextureCount] = true;
    }
    for (int i = 0; i < game->zombiesRemaining; i++) {
        Vector3 pos = GetZombieSpawnPoint();
        ZombieType type = ZOMBIE_TYPE_DEFAULT;
        int texIdx = 0;
        if (game->zombieMode == ZOMBIE_MODE_ALL_IMAGES && game->zombieHeadTextureCount > 0) {
            type = ZOMBIE_TYPE_IMAGE_HEAD;
            texIdx = i % game->zombieHeadTextureCount;
        }
        SpawnZombie(game, pos, type, texIdx);
    }
}

void GameInit(Game *game, int screenWidth, int screenHeight) {
    DebugInit(&game->debug, 10.0f);
    DebugLog(&game->debug, "Game started", DEBUG_SUCCESS);
    game->state = GAME_STATE_PLAYING;
    game->score = 0;
    game->totalDeadZombies = 0;
    game->round = 0;
    game->nonImageDeathsSinceLastImage = 0;
    game->gameTime = 0.0f;
    game->firstShotFired = false;
    game->firstShotGraceTimer = 0.3f;
    game->zombieCount = 0;
    game->particleCount = 0;
    game->bloodDecalCount = 0;
    game->zombieHeadTextureCount = 0;
    game->muzzleFlashTimer = 0.0f;
    game->muzzleFlashPos = (Vector3){ 0 };
    
    RendererInit(game, screenWidth, screenHeight);
    PlayerInit(&game->player, (Vector3){ 0, 1.5f, 0 }, game->shaders.pbr, &game->textures);
    WeaponInit(&game->weapon, game->shaders.pbr);
    CameraInit(&game->camera, &game->player);
    
    AudioInit(&game->audio);
    TextureGenerate(&game->textures);
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
    
    if (game->screenshotMode == SCREENSHOT_MODE_WORLD) {
        game->zombieCount = 0;
        game->zombiesRemaining = 0;
    } else if (game->screenshotMode == SCREENSHOT_MODE_ZOMBIE) {
        game->zombieCount = 0;
        game->zombiesRemaining = 0;
        SpawnZombie(game, (Vector3){ 0, 0, 2.0f }, ZOMBIE_TYPE_DEFAULT, 0);
        CameraSetMode(&game->camera, GAME_CAMERA_MODE_THIRD_PERSON);
        game->camera.baseMode = GAME_CAMERA_MODE_THIRD_PERSON;
    } else if (game->screenshotMode == SCREENSHOT_MODE_PLAYER) {
        game->zombieCount = 0;
        game->zombiesRemaining = 0;
        CameraSetMode(&game->camera, GAME_CAMERA_MODE_THIRD_PERSON);
        game->camera.baseMode = GAME_CAMERA_MODE_THIRD_PERSON;
    } else {
        SpawnWave(game);
    }
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
    
    Vector3 wallCenter = { 0, 1.2f, -10.0f };
    Vector3 wallSize = { 60.0f, 2.4f, 0.6f };
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
    
    for (int bx = -2; bx <= 2; bx++) {
        for (int bz = 0; bz <= 3; bz++) {
            float baseX = bx * 12.0f;
            float baseZ = bz * 14.0f + 6.0f;
            if (fabsf(baseX) < 1.5f && baseZ < 2.0f) continue;
            float h = 3.0f + ((bx + bz) % 4) * 1.8f;
            Vector3 bldPos = { baseX, h * 0.5f, baseZ };
            Vector3 bldSize = { 5.5f, h, 5.0f };
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
    }
    
    for (int i = 0; i < 6; i++) {
        float x = -10.0f + i * 3.5f;
        float z = -6.0f - (i % 2) * 1.0f;
        Vector3 containerPos = { x, 0.6f, z };
        Vector3 containerSize = { 2.4f, 1.2f, 6.0f };
        if (PointInAABB(p, containerPos, containerSize)) {
            float dx = p.x - containerPos.x;
            float dz = p.z - containerPos.z;
            float halfX = containerSize.x * 0.5f + radius;
            float halfZ = containerSize.z * 0.5f + radius;
            if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                game->player.position.x = containerPos.x + (dx > 0 ? halfX : -halfX);
            } else {
                game->player.position.z = containerPos.z + (dz > 0 ? halfZ : -halfZ);
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        float x = -10.0f + i * 3.5f;
        float z = -10.0f;
        Vector3 containerPos = { x, 0.6f, z };
        Vector3 containerSize = { 2.4f, 1.2f, 6.0f };
        if (PointInAABB(p, containerPos, containerSize)) {
            float dx = p.x - containerPos.x;
            float dz = p.z - containerPos.z;
            float halfX = containerSize.x * 0.5f + radius;
            float halfZ = containerSize.z * 0.5f + radius;
            if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                game->player.position.x = containerPos.x + (dx > 0 ? halfX : -halfX);
            } else {
                game->player.position.z = containerPos.z + (dz > 0 ? halfZ : -halfZ);
            }
        }
    }
    
    Vector3 fenceCenter = { 0, 0.6f, 8.5f };
    Vector3 fenceSize = { 30.0f, 1.2f, 2.0f };
    if (PointInAABB(p, fenceCenter, fenceSize)) {
        float dx = p.x - fenceCenter.x;
        float dz = p.z - fenceCenter.z;
        float halfX = fenceSize.x * 0.5f + radius;
        float halfZ = fenceSize.z * 0.5f + radius;
        if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
            game->player.position.x = fenceCenter.x + (dx > 0 ? halfX : -halfX);
        } else {
            game->player.position.z = fenceCenter.z + (dz > 0 ? halfZ : -halfZ);
        }
    }
    
    Vector3 platformCenter = { 10.0f, 0.3f, 3.0f };
    Vector3 platformSize = { 4.5f, 0.6f, 1.5f };
    if (PointInAABB(p, platformCenter, platformSize)) {
        float dx = p.x - platformCenter.x;
        float dz = p.z - platformCenter.z;
        float halfX = platformSize.x * 0.5f + radius;
        float halfZ = platformSize.z * 0.5f + radius;
        if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
            game->player.position.x = platformCenter.x + (dx > 0 ? halfX : -halfX);
        } else {
            game->player.position.z = platformCenter.z + (dz > 0 ? halfZ : -halfZ);
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
    
    for (int i = 0; i < 8; i++) {
        float angle = i * PI * 0.5f + PI * 0.25f;
        float rad = 18.0f;
        Vector3 lampPos = { cosf(angle) * rad, 0.05f, sinf(angle) * rad };
        if (PointInAABB(p, lampPos, (Vector3){ 0.6f, 0.6f, 0.6f })) {
            float dx = p.x - lampPos.x;
            float dz = p.z - lampPos.z;
            float dist2 = sqrtf(dx*dx + dz*dz);
            if (dist2 > 0.001f) {
                float pushDist = 0.6f * 0.5f + radius;
                game->player.position.x = lampPos.x + (dx / dist2) * pushDist;
                game->player.position.z = lampPos.z + (dz / dist2) * pushDist;
            }
        }
    }
    
    if (p.x < -45.0f) game->player.position.x = -45.0f;
    if (p.x > 45.0f) game->player.position.x = 45.0f;
    if (p.z < -45.0f) game->player.position.z = -45.0f;
    if (p.z > 45.0f) game->player.position.z = 45.0f;
    if (p.y < 0.0f) game->player.position.y = 0.0f;
}

static void GameApplyZombieCollisions(Game *game) {
    float radius = 0.4f;
    for (int i = 0; i < game->zombieCount; i++) {
        if (!ZombieIsAlive(&game->zombies[i])) continue;
        Vector3 p = game->zombies[i].position;
        Vector3 wallCenter = { 0, 1.2f, -10.0f };
        Vector3 wallSize = { 60.0f, 2.4f, 0.6f };
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
        
        for (int bx = -2; bx <= 2; bx++) {
            for (int bz = 0; bz <= 3; bz++) {
                float baseX = bx * 12.0f;
                float baseZ = bz * 14.0f + 6.0f;
                if (fabsf(baseX) < 1.5f && baseZ < 2.0f) continue;
                float h = 3.0f + ((bx + bz) % 4) * 1.8f;
                Vector3 bldPos = { baseX, h * 0.5f, baseZ };
                Vector3 bldSize = { 5.5f, h, 5.0f };
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
        }
        
        for (int c = 0; c < 10; c++) {
            float x = -10.0f + c * 3.5f;
            float z = (c < 6) ? (-6.0f - (c % 2) * 1.0f) : (-10.0f);
            Vector3 containerPos = { x, 0.6f, z };
            Vector3 containerSize = { 2.4f, 1.2f, 6.0f };
            if (PointInAABB(p, containerPos, containerSize)) {
                float dx = p.x - containerPos.x;
                float dz = p.z - containerPos.z;
                float halfX = containerSize.x * 0.5f + radius;
                float halfZ = containerSize.z * 0.5f + radius;
                if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                    game->zombies[i].position.x = containerPos.x + (dx > 0 ? halfX : -halfX);
                } else {
                    game->zombies[i].position.z = containerPos.z + (dz > 0 ? halfZ : -halfZ);
                }
            }
        }
        
        Vector3 fenceCenter = { 0, 0.6f, 8.5f };
        Vector3 fenceSize = { 30.0f, 1.2f, 2.0f };
        if (PointInAABB(p, fenceCenter, fenceSize)) {
            float dx = p.x - fenceCenter.x;
            float dz = p.z - fenceCenter.z;
            float halfX = fenceSize.x * 0.5f + radius;
            float halfZ = fenceSize.z * 0.5f + radius;
            if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                game->zombies[i].position.x = fenceCenter.x + (dx > 0 ? halfX : -halfX);
            } else {
                game->zombies[i].position.z = fenceCenter.z + (dz > 0 ? halfZ : -halfZ);
            }
        }
        
        Vector3 platformCenter = { 10.0f, 0.3f, 3.0f };
        Vector3 platformSize = { 4.5f, 0.6f, 1.5f };
        if (PointInAABB(p, platformCenter, platformSize)) {
            float dx = p.x - platformCenter.x;
            float dz = p.z - platformCenter.z;
            float halfX = platformSize.x * 0.5f + radius;
            float halfZ = platformSize.z * 0.5f + radius;
            if (fabsf(dx) / halfX > fabsf(dz) / halfZ) {
                game->zombies[i].position.x = platformCenter.x + (dx > 0 ? halfX : -halfX);
            } else {
                game->zombies[i].position.z = platformCenter.z + (dz > 0 ? halfZ : -halfZ);
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
        
        for (int l = 0; l < 8; l++) {
            float angle = l * PI * 0.5f + PI * 0.25f;
            float rad = 18.0f;
            Vector3 lampPos = { cosf(angle) * rad, 0.05f, sinf(angle) * rad };
            if (PointInAABB(p, lampPos, (Vector3){ 0.6f, 0.6f, 0.6f })) {
                float dx = p.x - lampPos.x;
                float dz = p.z - lampPos.z;
                float dist2 = sqrtf(dx*dx + dz*dz);
                if (dist2 > 0.001f) {
                    float pushDist = 0.6f * 0.5f + radius;
                    game->zombies[i].position.x = lampPos.x + (dx / dist2) * pushDist;
                    game->zombies[i].position.z = lampPos.z + (dz / dist2) * pushDist;
                }
            }
        }
        
        if (p.x < -45.0f) game->zombies[i].position.x = -45.0f;
        if (p.x > 45.0f) game->zombies[i].position.x = 45.0f;
        if (p.z < -45.0f) game->zombies[i].position.z = -45.0f;
        if (p.z > 45.0f) game->zombies[i].position.z = 45.0f;
        if (p.y < 0.0f) game->zombies[i].position.y = 0.0f;
    }
}

void GameUpdate(Game *game, float dt, InputState *input) {
    DebugUpdate(&game->debug, dt);
    if (IsKeyPressed(KEY_F1)) DebugToggle(&game->debug);
    if (IsKeyPressed(KEY_F2)) DebugClear(&game->debug);
    
    if (game->state != GAME_STATE_PLAYING) return;
    
    game->gameTime += dt;
    if (game->firstShotGraceTimer > 0.0f) game->firstShotGraceTimer -= dt;
    PlayerUpdate(&game->player, input, dt);
    CameraUpdate(&game->camera, &game->player, dt);
    WeaponUpdate(&game->weapon, game->player.position, game->player.yaw, input, dt);
    RendererUpdate(game, dt);
    
    GameApplyCollisions(game);
    
    DebugLogf(&game->debug, DEBUG_INFO, "Player health: %.1f", game->player.health);
    DebugLogf(&game->debug, DEBUG_INFO, "Zombies alive: %d", game->zombieCount);
    
    bool invincible = game->gameTime < 8.0f;
    
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieUpdate(&game->zombies[i], game->player.position, dt, game->firstShotFired);
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
    
    if (game->screenshotMode == SCREENSHOT_MODE_NONE) {
        if (input->cameraTogglePressed) {
            CameraToggleMode(&game->camera);
        }
        
        if (input->mouseRightDown) {
            CameraSetMode(&game->camera, GAME_CAMERA_MODE_FIRST_PERSON);
        } else {
            CameraSetMode(&game->camera, game->camera.baseMode);
        }
        CameraSetCrouch(&game->camera, input->ctrlPressed);
        
        if (input->mouseLeftPressed && WeaponCanShoot(&game->weapon) && game->firstShotGraceTimer <= 0.0f) {
            WeaponShoot(&game->weapon);
            AudioPlayGunshot(&game->audio);
            game->firstShotFired = true;
            
            RayHitInfo hit = WeaponRaycast(&game->weapon, game->camera.camera, game->zombies, game->zombieCount);
            if (hit.hit) {
                ZombieTakeDamage(&game->zombies[hit.zombieIndex], WEAPON_DAMAGE);
                if (!ZombieIsAlive(&game->zombies[hit.zombieIndex])) {
                    game->score += 100;
                }
                game->totalDeadZombies += 1;
                if (game->zombies[hit.zombieIndex].type == ZOMBIE_TYPE_IMAGE_HEAD && game->zombieMode == ZOMBIE_MODE_MIXED) {
                    bool anyImageAlive = false;
                    for (int k = 0; k < game->zombieCount; k++) {
                        if (k != hit.zombieIndex && game->zombies[k].type == ZOMBIE_TYPE_IMAGE_HEAD && ZombieIsAlive(&game->zombies[k])) {
                            anyImageAlive = true;
                            break;
                        }
                    }
                    if (!anyImageAlive) {
                        for (int k = 0; k < game->zombieCount; k++) {
                            if (game->zombies[k].type == ZOMBIE_TYPE_DEFAULT && ZombieIsAlive(&game->zombies[k])) {
                                game->zombies[k].speed = ZOMBIE_SPEED_BASE * 2.0f;
                            }
                        }
                    }
                }
                if (game->mode == GAME_MODE_ENDLESS && game->zombieMode == ZOMBIE_MODE_MIXED && game->zombieHeadTextureCount > 0) {
                    bool anyImageAlive = false;
                    for (int k = 0; k < game->zombieCount; k++) {
                        if (game->zombies[k].type == ZOMBIE_TYPE_IMAGE_HEAD && ZombieIsAlive(&game->zombies[k])) {
                            anyImageAlive = true;
                            break;
                        }
                    }
                    if (!anyImageAlive) {
                        if (game->zombies[hit.zombieIndex].type == ZOMBIE_TYPE_DEFAULT) {
                            game->nonImageDeathsSinceLastImage++;
                        }
                        if (game->nonImageDeathsSinceLastImage >= 3) {
                            game->nonImageDeathsSinceLastImage = 0;
                            for (int k = 0; k < game->zombieCount; k++) {
                                if (game->zombies[k].type == ZOMBIE_TYPE_DEFAULT && ZombieIsAlive(&game->zombies[k])) {
                                    game->zombies[k].speed = ZOMBIE_SPEED_BASE;
                                }
                            }
                            int usedImages[16] = { false };
                            for (int k = 0; k < game->zombieCount; k++) {
                                if (game->zombies[k].type == ZOMBIE_TYPE_IMAGE_HEAD && game->zombies[k].active) {
                                    if (game->zombies[k].textureIndex >= 0 && game->zombies[k].textureIndex < 16) {
                                        usedImages[game->zombies[k].textureIndex] = true;
                                    }
                                }
                            }
                            int available[16];
                            int availCount = 0;
                            for (int j = 0; j < game->zombieHeadTextureCount && j < 16; j++) {
                                if (!usedImages[j]) available[availCount++] = j;
                            }
                            if (availCount > 0) {
                                int spawnCount = 1 + rand() % availCount;
                                int usedPick[16] = { false };
                                int picked = 0;
                                while (picked < spawnCount && picked < availCount) {
                                    int idx = rand() % availCount;
                                    if (!usedPick[idx]) {
                                        usedPick[idx] = true;
                                        SpawnImageZombie(game, available[idx]);
                                        picked++;
                                    }
                                }
                            }
                        }
                    }
                }
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
                            bloodVel, 2.5f, PARTICLE_BLOOD, 0.12f + rand()%100/800.0f, (Color){ 120, 20, 20, 255 });
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
                            3.0f, PARTICLE_BLOOD, 0.25f + rand()%100/500.0f, (Color){ 100, 15, 15, 220 });
                    }
                }
            }
            for (int p = 0; p < 5; p++) {
                if (game->particleCount < 256) {
                    ParticleSpawn(&game->particles[game->particleCount++], hit.point,
                        (Vector3){ (rand()%100-50)/50.0f, (rand()%100-50)/50.0f, (rand()%100-50)/50.0f },
                        1.0f, PARTICLE_BLOOD, 0.05f, (Color){ 100, 15, 15, 255 });
                }
            }
        }
        
        game->muzzleFlashPos = game->weapon.position;
        game->muzzleFlashTimer = 0.05f;
    }
    
    
    if (IsKeyPressed(KEY_R)) WeaponReload(&game->weapon);
    
    ParticleSystemUpdate(game->particles, game->particleCount, dt);
    
    if (game->muzzleFlashTimer > 0) game->muzzleFlashTimer -= dt;
    
    if (game->screenshotMode == SCREENSHOT_MODE_NONE) {
        bool allDead = true;
        for (int i = 0; i < game->zombieCount; i++) {
            if (ZombieIsAlive(&game->zombies[i])) { allDead = false; break; }
        }
        if (allDead) {
            SpawnWave(game);
        }
    }
}

void GameRender(Game *game) {
    Camera3D cam = CameraGetCamera(&game->camera);
    RendererBegin(game, cam);
    
    if (game->screenshotMode != SCREENSHOT_MODE_ZOMBIE && game->screenshotMode != SCREENSHOT_MODE_PLAYER) {
        RendererDrawScene(game);
    }
    
    if (game->screenshotMode != SCREENSHOT_MODE_PLAYER) {
        RendererDrawBloodDecals(game);
        RendererDrawZombies(game, game->shaders.pbr);
    }
    
    if (game->screenshotMode != SCREENSHOT_MODE_ZOMBIE) {
        if (CameraGetMode(&game->camera) == GAME_CAMERA_MODE_THIRD_PERSON) {
            RendererDrawPlayer(&game->player, game->shaders.pbr);
            WeaponRender(&game->weapon, cam, game->player.yaw);
        } else {
            WeaponRenderFirstPerson(&game->weapon, cam, game->player.yaw);
        }
    }
    
    RendererDrawParticles(game->particles, game->particleCount);
    RendererEnd(game);
    RendererDrawZombieHeads(game);
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
    game.menu.active = true;
    game.state = GAME_STATE_MENU;
    
    InputState input;
    int frameCount = 0;
    char screenshotPath[256] = { 0 };
    const char *shotEnv = getenv("ZOMBIE_SHOT");
    if (shotEnv) {
        snprintf(screenshotPath, sizeof(screenshotPath), "%s", shotEnv);
    } else {
        int nextIter = FindNextIteration("../workflow");
        snprintf(screenshotPath, sizeof(screenshotPath), "../workflow/iteration_%02d.png", nextIter);
    }
    int autoQuitMs = -1;
    const char *autoQuitStr = getenv("ZOMBIE_AUTO_QUIT_MS");
    if (autoQuitStr) autoQuitMs = atoi(autoQuitStr);
    double startTime = GetTime();
    bool autoStart = getenv("ZOMBIE_AUTO_START") != NULL;
    int autoStartFrame = 30;
    int screenshotFrame = 35;
    
    bool autoRotate = getenv("ZOMBIE_SCREENSHOT_ROTATE") != NULL;
    int autoRotateStage = 0;
    int autoRotateBase = FindNextIteration("../workflow");
    float autoRotateAccum = 0.0f;
    float lastYaw = 0.0f;
    const float autoRotateSpeed = 9.5f;
    float autoRotateAngleThreshold = PI * 0.5f;
    int autoRotateMaxScreenshots = 4;
    const char *stepEnv = getenv("ZOMBIE_SCREENSHOT_STEP_DEGREES");
    if (stepEnv) {
        float stepDeg = atof(stepEnv);
        if (stepDeg > 0.1f && stepDeg <= 180.0f) {
            autoRotateAngleThreshold = stepDeg * PI / 180.0f;
            autoRotateMaxScreenshots = (int)(360.0f / stepDeg);
            if (autoRotateMaxScreenshots < 1) autoRotateMaxScreenshots = 1;
        }
    }
    
    game.screenshotMode = SCREENSHOT_MODE_NONE;
    if (getenv("ZOMBIE_SCREENSHOT_WORLD") != NULL) {
        game.screenshotMode = SCREENSHOT_MODE_WORLD;
    } else if (getenv("ZOMBIE_SCREENSHOT_ZOMBIE") != NULL) {
        game.screenshotMode = SCREENSHOT_MODE_ZOMBIE;
    } else if (getenv("ZOMBIE_SCREENSHOT_PLAYER") != NULL) {
        game.screenshotMode = SCREENSHOT_MODE_PLAYER;
    }
    
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
            game.menu.active = false;
            GameInit(&game, screenWidth, screenHeight);
        }
        
        if (autoRotate && game.state == GAME_STATE_PLAYING && !game.menu.active && autoRotateStage < autoRotateMaxScreenshots) {
            fprintf(stderr, "AUTO_ROTATE: stage=%d yaw=%.3f accum=%.3f\n", autoRotateStage, game.player.yaw, autoRotateAccum);
            input.mouseDelta.x = autoRotateSpeed;
            input.mouseDelta.y = 0.0f;
        }
        
        if (game.state == GAME_STATE_PLAYING && !game.menu.active) {
            GameUpdate(&game, GetFrameTime(), &input);
        }
        
        if (autoRotate && game.state == GAME_STATE_PLAYING && !game.menu.active && autoRotateStage < autoRotateMaxScreenshots) {
            float currentYaw = game.player.yaw;
            float deltaYaw = currentYaw - lastYaw;
            autoRotateAccum += deltaYaw;
            lastYaw = currentYaw;
            
            while (autoRotateAccum <= -autoRotateAngleThreshold) {
                autoRotateAccum += autoRotateAngleThreshold;
                char path[256];
                snprintf(path, sizeof(path), "../workflow/iteration_%02d.png", autoRotateBase + autoRotateStage);
                fprintf(stderr, "AUTO_ROTATE: Taking screenshot %d -> %s\n", autoRotateStage, path);
                TakeScreenshot(path);
                autoRotateStage++;
            }
            
            if (autoRotateStage >= autoRotateMaxScreenshots) {
                break;
            }
        }
        
        BeginDrawing();
        ClearBackground((Color){ 245, 240, 232, 255 });
        
        if (game.state == GAME_STATE_MENU || game.menu.active) {
            UIRender(&game.menu);
            if (game.debug.enabled) DebugRender(&game.debug, screenWidth, screenHeight);
        } else if (game.state == GAME_STATE_PLAYING) {
            GameRender(&game);
        } else if (game.state == GAME_STATE_GAMEOVER) {
            DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 20, 40, (Color){ 100, 30, 30, 255 });
            DrawText(TextFormat("Final Score: %d", game.score), screenWidth / 2 - MeasureText(TextFormat("Final Score: %d", game.score), 20) / 2, screenHeight / 2 + 30, 20, (Color){ 35, 35, 40, 255 });
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
