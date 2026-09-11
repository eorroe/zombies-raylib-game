#include "renderer.h"
#include "texture.h"
#include "shader.h"
#include "particle.h"
#include "zombie.h"
#include "raymath.h"
#include <stdio.h>

static void SetModelTexture(Model *model, Texture2D diffuse, Texture2D normal) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = diffuse;
        if (normal.id != 0) {
            model->materials[0].maps[MATERIAL_MAP_NORMAL].texture = normal;
        }
    }
}

static void SetPBRMaterial(Game *game, Texture2D albedo, Texture2D normal, float metallic, float roughness) {
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocAlbedo, &albedo, SHADER_UNIFORM_SAMPLER2D);
    if (normal.id != 0) {
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocNormal, &normal, SHADER_UNIFORM_SAMPLER2D);
    }
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocMetallic, &metallic, SHADER_UNIFORM_FLOAT);
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocRoughness, &roughness, SHADER_UNIFORM_FLOAT);
}

static Vector3 fireLightPositions[4];
static bool fireLightsInitialized = false;

void RendererInit(Game *game, int screenWidth, int screenHeight) {
    game->sceneTarget = LoadRenderTexture(screenWidth, screenHeight);
    game->postProcessTarget = LoadRenderTexture(screenWidth, screenHeight);
    ShaderInit(&game->shaders, screenWidth, screenHeight);
    ShaderUpdate(&game->shaders, 0.0f, screenWidth, screenHeight);
    
    Mesh crateMesh = GenMeshCube(0.6f, 0.6f, 0.6f);
    game->crateModel = LoadModelFromMesh(crateMesh);
    if (game->textures.generated && game->textures.wood.id != 0) {
        SetModelTexture(&game->crateModel, game->textures.wood, game->textures.crateNormal);
    }
    game->crateModel.materials[0].shader = game->shaders.pbr;
    
    Mesh barrelMesh = GenMeshCylinder(0.2f, 0.8f, 16);
    game->barrelModel = LoadModelFromMesh(barrelMesh);
    if (game->textures.generated && game->textures.metal.id != 0) {
        SetModelTexture(&game->barrelModel, game->textures.metal, game->textures.metalNormal);
    }
    game->barrelModel.materials[0].shader = game->shaders.pbr;
    
    Mesh wallMesh = GenMeshCube(4.5f, 1.5f, 0.4f);
    game->wallModel = LoadModelFromMesh(wallMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->wallModel, game->textures.concrete, game->textures.concreteNormal);
    }
    game->wallModel.materials[0].shader = game->shaders.pbr;
    
    Mesh buildingMesh = GenMeshCube(2.5f, 3.0f, 2.5f);
    game->buildingModel = LoadModelFromMesh(buildingMesh);
    if (game->textures.generated && game->textures.brick.id != 0) {
        SetModelTexture(&game->buildingModel, game->textures.brick, (Texture2D){0});
    }
    game->buildingModel.materials[0].shader = game->shaders.pbr;
    
    Mesh trainMesh = GenMeshCube(12.0f, 3.5f, 3.0f);
    game->trainModel = LoadModelFromMesh(trainMesh);
    if (game->textures.generated && game->textures.metal.id != 0) {
        SetModelTexture(&game->trainModel, game->textures.metal, game->textures.metalNormal);
    }
    game->trainModel.materials[0].shader = game->shaders.pbr;
    
    Mesh floorMesh = GenMeshPlane(50, 50, 50, 50);
    game->floorModel = LoadModelFromMesh(floorMesh);
    if (game->textures.generated && game->textures.asphalt.id != 0) {
        SetModelTexture(&game->floorModel, game->textures.asphalt, (Texture2D){0});
    }
    game->floorModel.materials[0].shader = game->shaders.pbr;
    
    Mesh bloodMesh = GenMeshPlane(1.5f, 1.5f, 4, 4);
    game->bloodDecalModel = LoadModelFromMesh(bloodMesh);
    if (game->textures.generated && game->textures.bloodDecal.id != 0) {
        SetModelTexture(&game->bloodDecalModel, game->textures.bloodDecal, (Texture2D){0});
    }
    game->bloodDecalModel.materials[0].shader = game->shaders.pbr;
    
    Mesh fencePostMesh = GenMeshCylinder(0.08f, 1.2f, 8);
    game->fenceModel = LoadModelFromMesh(fencePostMesh);
    if (game->textures.generated && game->textures.fence.id != 0) {
        SetModelTexture(&game->fenceModel, game->textures.fence, game->textures.metalNormal);
    }
    game->fenceModel.materials[0].shader = game->shaders.pbr;
    
    Mesh containerMesh = GenMeshCube(2.4f, 1.2f, 6.0f);
    game->containerModel = LoadModelFromMesh(containerMesh);
    if (game->textures.generated && game->textures.container.id != 0) {
        SetModelTexture(&game->containerModel, game->textures.container, game->textures.containerNormal);
    }
    game->containerModel.materials[0].shader = game->shaders.pbr;
    
    Mesh platformMesh = GenMeshCube(4.0f, 0.6f, 1.5f);
    game->platformModel = LoadModelFromMesh(platformMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->platformModel, game->textures.concrete, game->textures.concreteNormal);
    }
    game->platformModel.materials[0].shader = game->shaders.pbr;
    
    Mesh rubbleMesh = GenMeshCube(0.3f, 0.15f, 0.3f);
    game->rubbleModel = LoadModelFromMesh(rubbleMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->rubbleModel, game->textures.concrete, (Texture2D){0});
    }
    game->rubbleModel.materials[0].shader = game->shaders.pbr;
    
    fireLightPositions[0] = (Vector3){ -4.0f, 1.5f, 12.0f };
    fireLightPositions[1] = (Vector3){ 0.0f, 0.5f, 2.0f };
    fireLightPositions[2] = (Vector3){ 8.0f, 2.0f, 8.0f };
    fireLightPositions[3] = (Vector3){ -8.0f, 0.3f, -4.0f };
    fireLightsInitialized = true;
}

void RendererUpdate(Game *game, float dt) {
    (void)dt;
    float t = GetTime();
    for (int i = 0; i < 4; i++) {
        float flicker = 1.0f + sinf(t * 8.0f + i * 2.5f) * 0.15f + sinf(t * 13.0f + i * 1.7f) * 0.1f;
        fireLightPositions[i].y += sinf(t * 3.0f + i) * 0.02f;
    }
    
    ShaderSetFog(&game->shaders, (Vector3){ 0.3f, 0.4f, 0.7f }, 0.001f);
}

void RendererBegin(Game *game, Camera3D camera) {
    BeginTextureMode(game->sceneTarget);
    ClearBackground(BLACK);
    
<<<<<<< Updated upstream
    int w = 1280;
    int h = 720;
    
    BeginTextureMode(game->sceneTarget);
    ClearBackground((Color){ 245, 240, 232, 255 });
=======
    int w = game->sceneTarget.texture.width;
    int h = game->sceneTarget.texture.height;
    DrawRectangleGradientV(0, 0, w, h / 2, (Color){ 120, 160, 220, 255 }, (Color){ 220, 180, 120, 255 });
    DrawRectangleGradientV(0, h / 2, w, h / 2, (Color){ 220, 180, 120, 255 }, (Color){ 180, 120, 80, 255 });
>>>>>>> Stashed changes
    
    BeginMode3D(camera);
    rlDisableBackfaceCulling();
}

void RendererDrawScene(Game *game) {
    (void)game;

    DrawGrid(100, 1.0f);

    for (int sx = -48; sx <= 48; sx += 12) {
    }

    for (int sz = -38; sz <= 48; sz += 14) {
        if (fabsf(sz) < 4.0f) continue;
    }

    for (int cx = -42; cx <= 42; cx += 12) {
    }
<<<<<<< Updated upstream

    for (int i = 0; i < 24; i++) {
        float angle = i * PI * 0.25f;
        float radius = 6.0f + (i % 3) * 4.0f;
        Vector3 cratePos = {
            cosf(angle) * radius,
            0.3f,
            sinf(angle) * radius
        };
        DrawCubeWires(cratePos, 1.0f, 1.0f, 1.0f, (Color){ 20, 30, 60, 255 });
=======
    for (int i = 0; i < 4; i++) {
        float x = -10.0f + i * 3.5f;
        Vector3 containerPos = { x, 0.6f, -10.0f };
        DrawModel(game->containerModel, containerPos, 1.0f, (Color){ 50, 90, 150, 255 });
>>>>>>> Stashed changes
    }

    for (int i = 0; i < 14; i++) {
        float angle = i * PI * 0.5f + 0.3f;
        float radius = 4.0f + (i % 2) * 4.0f;
        Vector3 barrelPos = {
            cosf(angle) * radius,
            0.4f,
            sinf(angle) * radius
        };
        DrawCylinderWires(barrelPos, 0.3f, 0.3f, 1.2f, 12, (Color){ 20, 30, 60, 255 });
    }

    for (int bx = -2; bx <= 2; bx++) {
        for (int bz = 0; bz <= 3; bz++) {
            float baseX = bx * 12.0f;
            float baseZ = bz * 14.0f + 6.0f;
            if (fabsf(baseX) < 1.5f && baseZ < 2.0f) continue;

            float h = 3.0f + ((bx + bz) % 4) * 1.8f;

            DrawCubeWires((Vector3){ baseX, h * 0.5f, baseZ }, 5.5f, h, 5.0f, (Color){ 20, 30, 60, 255 });

            for (int wy = -2; wy <= 2; wy++) {
                for (int wx = -2; wx <= 2; wx++) {
                    if ((wx + wy) % 2 == 0) continue;
                    float winX = baseX + wx * 0.9f;
                    float winZ = baseZ - 2.56f;
                    float winY = wy * 1.1f;
                    DrawCubeWires((Vector3){ winX, winY, winZ }, 0.6f, 0.8f, 0.05f, (Color){ 20, 30, 60, 255 });

                    float awningZ = baseZ - 2.6f;
                    float awningY = winY - 0.5f;
                    if ((wx + wy) % 3 == 0) {
                        DrawCubeWires((Vector3){ winX, awningY, awningZ }, 0.7f, 0.08f, 0.4f, (Color){ 20, 30, 60, 255 });
                    }
                }
            }

            if (h > 5.0f) {
                for (int wy = -1; wy <= 1; wy++) {
                    float fireX = baseX - 2.0f;
                    float fireZ = baseZ + 2.56f;
                    float fireY = wy * 1.5f;
                    DrawCubeWires((Vector3){ fireX, fireY, fireZ }, 0.08f, 0.8f, 0.4f, (Color){ 20, 30, 60, 255 });
                    DrawCubeWires((Vector3){ fireX - 0.2f, fireY + 0.2f, fireZ }, 0.25f, 0.08f, 0.35f, (Color){ 20, 30, 60, 255 });
                }
            }

            DrawCubeWires((Vector3){ baseX, h + 0.15f, baseZ }, 5.7f, 0.3f, 5.2f, (Color){ 20, 30, 60, 255 });

            if ((bx + bz) % 2 == 0) {
                DrawCubeWires((Vector3){ baseX, h + 0.4f, baseZ }, 1.0f, 0.5f, 0.8f, (Color){ 20, 30, 60, 255 });
            }

            for (int w = 0; w < 3; w++) {
                float sx = baseX - 1.4f + w * 1.4f;
                float sy = -h * 0.35f;
                DrawCubeWires((Vector3){ sx, sy, baseZ + 2.56f }, 0.7f, 1.1f, 0.05f, (Color){ 20, 30, 60, 255 });
            }
        }
    }

    Vector3 wallPos = { 0, 1.2f, -10.0f };
    DrawCubeWires(wallPos, 60.0f, 2.4f, 0.6f, (Color){ 20, 30, 60, 255 });

    for (int i = 0; i < 8; i++) {
        float angle = i * PI * 0.5f + PI * 0.25f;
        float radius = 18.0f;
        Vector3 lampPos = {
            cosf(angle) * radius,
            2.0f,
            sinf(angle) * radius
        };
        DrawCylinderWires(lampPos, 0.2f, 0.2f, 3.0f, 8, (Color){ 20, 30, 60, 255 });
        DrawSphereWires((Vector3){ lampPos.x, lampPos.y + 1.5f, lampPos.z }, 0.5f, 8, 8, (Color){ 20, 30, 60, 255 });
    }

    for (int i = 0; i < 6; i++) {
        float x = -20.0f + i * 8.0f;
        DrawCubeWires((Vector3){ x, 0.25f, 4.5f }, 0.6f, 0.5f, 0.6f, (Color){ 20, 30, 60, 255 });
        DrawCylinderWires((Vector3){ x, 0.55f, 4.5f }, 0.08f, 0.08f, 0.6f, 8, (Color){ 20, 30, 60, 255 });
    }

    for (int i = 0; i < 4; i++) {
<<<<<<< Updated upstream
        float x = -16.0f + i * 10.0f;
        DrawCubeWires((Vector3){ x, 1.2f, 4.5f }, 0.08f, 1.4f, 0.08f, (Color){ 20, 30, 60, 255 });
        DrawCubeWires((Vector3){ x + 0.2f, 1.6f, 4.5f }, 0.6f, 0.4f, 0.05f, (Color){ 20, 30, 60, 255 });
=======
        float t = GetTime();
        for (int p = 0; p < 25; p++) {
            float ft = t * 3.0f + p * 1.7f + i * 5.0f;
            float fx = fireLightPositions[i].x + sinf(ft * 2.3f) * 0.4f;
            float fy = fireLightPositions[i].y + fmodf(ft * 0.8f, 1.5f) + 0.3f;
            float fz = fireLightPositions[i].z + cosf(ft * 1.9f) * 0.4f;
            float alpha = 0.6f - fmodf(ft * 0.8f, 1.5f) * 0.4f;
            if (alpha < 0.0f) alpha = 0.0f;
            float size = 0.06f + fmodf(ft, 1.0f) * 0.1f;
            DrawSphere((Vector3){ fx, fy, fz }, size, (Color){ 100, 150, 220, (unsigned char)(alpha * 255) });
        }
    }
    EndBlendMode();
    
    {
        Vector3 wallPos = { 0, 0.75f, -10.0f };
        DrawModel(game->wallModel, wallPos, 1.0f, WHITE);
    }
    
    for (int i = 0; i < 180; i++) {
        float t = GetTime() * 0.5f + i * 3.31f;
        int fireIdx = i % 4;
        Vector3 basePos = fireLightPositions[fireIdx];
        float x = basePos.x + sinf(t * 1.7f + fireIdx * 2.0f) * 1.2f;
        float y = basePos.y + fmodf(t * 0.4f, 2.5f) + 0.2f;
        float z = basePos.z + cosf(t * 1.3f + fireIdx * 2.0f) * 1.2f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.25f + sinf(t + i) * 0.15f;
        DrawSphere(dustPos, 0.12f, (Color){ 150, 180, 220, (unsigned char)(alpha * 255) });
    }
    
    for (int i = 0; i < 250; i++) {
        float t = GetTime() * 0.25f + i * 2.17f;
        float x = sinf(t * 1.1f) * 26.0f;
        float y = 0.4f + fmodf(t * 0.5f, 5.0f);
        float z = cosf(t * 0.8f) * 26.0f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.4f + sinf(t + i) * 0.25f;
        DrawSphere(dustPos, 0.18f, (Color){ 180, 200, 230, (unsigned char)(alpha * 255) });
>>>>>>> Stashed changes
    }
}

void RendererDrawBloodDecals(Game *game) {
    if (game->bloodDecalCount <= 0) return;
    for (int i = 0; i < game->bloodDecalCount; i++) {
        Vector3 pos = game->bloodDecals[i];
        pos.y = 0.03f;
<<<<<<< Updated upstream
        DrawPlane(pos, (Vector2){ 5.0f, 5.0f }, (Color){ 40, 60, 120, 200 });
=======
        DrawPlane(pos, (Vector2){ 5.0f, 5.0f }, (Color){ 60, 90, 160, 200 });
>>>>>>> Stashed changes
    }
}

void RendererDrawZombies(Game *game, Shader shader) {
    (void)shader;
    printf("ZOMBIES: count=%d\n", game->zombieCount);
    Camera3D cam = CameraGetCamera(&game->camera);
    ShaderBeginPBR(&game->shaders);
    ShaderSetFog(&game->shaders, (Vector3){ 0.3f, 0.4f, 0.7f }, 0.001f);
    ShaderSetDirectionalLight(&game->shaders, (Vector3){ 0.5f, 0.8f, 0.3f }, (Vector3){ 0.8f, 0.9f, 1.2f });
    Vector3 lightPositions[4];
    Vector3 lightColors[4];
    float t = GetTime();
    for (int i = 0; i < 4; i++) {
        lightPositions[i] = fireLightPositions[i];
        float flicker = 1.0f + sinf(t * 8.0f + i * 2.5f) * 0.15f + sinf(t * 13.0f + i * 1.7f) * 0.1f;
        lightColors[i] = (Vector3){ 1.2f * flicker, 0.7f * flicker, 0.4f * flicker };
    }
    int lightCount = 4;
    for (int i = 0; i < 4; i++) {
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightPos[i], &lightPositions[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCol[i], &lightColors[i], SHADER_UNIFORM_VEC3);
    }
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCount, &lightCount, SHADER_UNIFORM_INT);
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieRender(&game->zombies[i], cam, game->zombieHeadTextures, game->zombieHeadTextureCount, game->shaders.pbr);
    }
    ShaderEnd(&game->shaders);
}

void RendererDrawZombieHeads(Game *game) {
    Camera3D cam = CameraGetCamera(&game->camera);
    for (int i = 0; i < game->zombieCount; i++) {
        Zombie *z = &game->zombies[i];
        if (!z->active) continue;
        
        float bob = sinf(z->animTime) * 0.05f;
        float headY = z->position.y + bob + LEG_UPPER_LEN + LEG_LOWER_LEN + TORSO_HEIGHT + HEAD_RADIUS * 0.9f;
        Vector3 headPos = (Vector3){ z->position.x, headY, z->position.z };
        
        if (z->type == ZOMBIE_TYPE_IMAGE_HEAD && game->zombieHeadTextureCount > 0 && z->textureIndex >= 0 && z->textureIndex < game->zombieHeadTextureCount) {
            Texture2D tex = game->zombieHeadTextures[z->textureIndex];
            if (tex.id == 0) continue;
            
            if (z->dying) {
                float deathProgress = 1.0f - (z->deathTimer / 1.0f);
                if (deathProgress > 1.0f) deathProgress = 1.0f;
                Vector3 groundHeadPos = (Vector3){ z->position.x, 0.15f, z->position.z };
                Vector2 screenPos = GetWorldToScreen(groundHeadPos, cam);
                float size = 120.0f * 8.0f / Vector3Length(Vector3Subtract(cam.position, groundHeadPos));
                Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
                Rectangle dst = { screenPos.x - size / 2, screenPos.y - size / 2, size, size };
                DrawTexturePro(tex, src, dst, (Vector2){ 0, 0 }, 90.0f, WHITE);
            } else {
                Vector2 screenPos = GetWorldToScreen(headPos, cam);
                float dist = Vector3Length(Vector3Subtract(cam.position, headPos));
                float size = 120.0f * 8.0f / dist;
                Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
                Rectangle dst = { screenPos.x - size / 2, screenPos.y - size / 2, size, size };
                DrawTexturePro(tex, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
                
                float barHeight = 6.0f;
                float barWidth = size;
                float barX = screenPos.x - size / 2;
                float barY = screenPos.y - size / 2 - barHeight - 4.0f;
                float healthPct = (z->maxHealth > 0.0f) ? (z->health / z->maxHealth) : 0.0f;
<<<<<<< Updated upstream
                DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 40, 60, 120, 255 });
                DrawRectangle(barX, barY, barWidth * healthPct, barHeight, (Color){ 80, 120, 180, 255 });
=======
                DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 40, 70, 130, 255 });
                DrawRectangle(barX, barY, barWidth * healthPct, barHeight, (Color){ 100, 150, 220, 255 });
>>>>>>> Stashed changes
            }
        } else {
            Vector2 screenPos = GetWorldToScreen(headPos, cam);
            float barWidth = 40.0f;
            float barHeight = 4.0f;
            float healthPct = (z->maxHealth > 0.0f) ? (z->health / z->maxHealth) : 0.0f;
            DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth, barHeight, (Color){ 40, 60, 120, 255 });
            DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth * healthPct, barHeight, (Color){ 80, 120, 180, 255 });
        }
    }
}

void RendererDrawPlayer(Player *player, Shader shader) {
    (void)shader;
    PlayerRender(player, shader);
}

void RendererDrawParticles(Particle *particles, int count) {
    ParticleSystemRender(particles, count);
    
    for (int i = 0; i < count; i++) {
        if (particles[i].type == PARTICLE_BLOOD) {
            DrawSphere(particles[i].position, particles[i].size * 4.0f, particles[i].color);
            DrawSphereWires(particles[i].position, particles[i].size * 4.0f, 8, 8, (Color){ 20, 30, 60, 180 });
        }
    }
    
    for (int i = 0; i < 500; i++) {
        float t = GetTime() * 0.4f + i * 2.17f;
        float x = sinf(t * 1.3f) * 28.0f;
        float y = 0.8f + fmodf(t * 0.8f, 8.0f);
        float z = cosf(t * 0.9f) * 28.0f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.8f + sinf(t + i) * 0.2f;
        DrawSphere(dustPos, 0.3f, (Color){ 100, 130, 170, (unsigned char)(alpha * 255) });
        DrawSphereWires(dustPos, 0.3f, 8, 8, (Color){ 20, 30, 60, (unsigned char)(alpha * 180) });
    }
}

void RendererDrawHUD(Game *game) {
    DrawRectangle(0, 0, game->sceneTarget.texture.width, 60, (Color){ 230, 235, 240, 200 });
    DrawRectangleLines(0, 0, game->sceneTarget.texture.width, 60, (Color){ 20, 30, 60, 255 });
    int x = 20;
    int gap = 10;
    int y = 20;
    int fontSize = 30;
    Color color = (Color){ 20, 30, 60, 255 };

    const char *scoreText = TextFormat("Score: %d", game->score);
    DrawText(scoreText, x, y, fontSize, color);
    x += MeasureText(scoreText, fontSize) + gap;

    const char *killsText = TextFormat("Kills: %d", game->totalDeadZombies);
    DrawText(killsText, x, y, fontSize, color);
    x += MeasureText(killsText, fontSize) + gap;

    const char *healthText = TextFormat("Health: %.0f", game->player.health);
    DrawText(healthText, x, y, fontSize, color);
    x += MeasureText(healthText, fontSize) + gap;

    const char *ammoText = TextFormat("Ammo: %d", game->weapon.ammo);
    DrawText(ammoText, x, y, fontSize, color);
    x += MeasureText(ammoText, fontSize) + gap;

    const char *modeText = TextFormat("Mode: %s", (game->mode == GAME_MODE_ROUNDS) ? "Rounds" : "Endless");
    DrawText(modeText, x, y, fontSize, color);
    x += MeasureText(modeText, fontSize) + gap;

    if (game->mode == GAME_MODE_ROUNDS) {
        const char *roundText = TextFormat("Round: %d", game->round);
        DrawText(roundText, x, y, fontSize, color);
        x += MeasureText(roundText, fontSize) + gap;
    }

    if (game->weapon.reloading) {
        DrawText("RELOADING...", x, y, fontSize, (Color){ 120, 40, 40, 255 });
    }
    
    if (CameraGetFirstPersonBlend(&game->camera) > 0.5f) {
        int cx = game->sceneTarget.texture.width / 2;
        int cy = game->sceneTarget.texture.height / 2;
        DrawLine(cx - 10, cy, cx + 10, cy, (Color){ 40, 80, 160, 255 });
        DrawLine(cx, cy - 10, cx, cy + 10, (Color){ 40, 80, 160, 255 });
    }
}

void RendererDrawScope(Game *game) {
    (void)game;
}

void RendererEnd(Game *game) {
    EndMode3D();
    EndTextureMode();
    
<<<<<<< Updated upstream
    DrawTextureRec(game->sceneTarget.texture, (Rectangle){ 0, 0, (float)game->sceneTarget.texture.width, (float)-game->sceneTarget.texture.height }, (Vector2){ 0, 0 }, WHITE);
    
    BeginTextureMode(game->postProcessTarget);
    ClearBackground((Color){ 0, 0, 0, 0 });
    BeginShaderMode(game->shaders.postProcess);
    DrawTextureRec(game->sceneTarget.texture, (Rectangle){ 0, 0, (float)game->sceneTarget.texture.width, (float)-game->sceneTarget.texture.height }, (Vector2){ 0, 0 }, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    DrawTextureRec(game->postProcessTarget.texture, (Rectangle){ 0, 0, (float)game->postProcessTarget.texture.width, (float)-game->postProcessTarget.texture.height }, (Vector2){ 0, 0 }, WHITE);
=======
    ShaderUpdate(&game->shaders, GetTime(), 1280, 720);
    ShaderBeginPostProcess(&game->shaders);
    int w = game->sceneTarget.texture.width;
    int h = game->sceneTarget.texture.height;
    DrawTexturePro(game->sceneTarget.texture,
        (Rectangle){ 0, 0, (float)w, (float)-h },
        (Rectangle){ 0, 0, (float)w, (float)h },
        (Vector2){ 0, 0 }, 0.0f, WHITE);
    ShaderEnd(&game->shaders);
>>>>>>> Stashed changes
}

void RendererShutdown(Game *game) {
    UnloadRenderTexture(game->sceneTarget);
    UnloadRenderTexture(game->postProcessTarget);
    if (game->crateModel.meshCount > 0) UnloadModel(game->crateModel);
    if (game->barrelModel.meshCount > 0) UnloadModel(game->barrelModel);
    if (game->wallModel.meshCount > 0) UnloadModel(game->wallModel);
    if (game->buildingModel.meshCount > 0) UnloadModel(game->buildingModel);
    if (game->trainModel.meshCount > 0) UnloadModel(game->trainModel);
    if (game->floorModel.meshCount > 0) UnloadModel(game->floorModel);
    if (game->bloodDecalModel.meshCount > 0) UnloadModel(game->bloodDecalModel);
    if (game->fenceModel.meshCount > 0) UnloadModel(game->fenceModel);
    if (game->containerModel.meshCount > 0) UnloadModel(game->containerModel);
    if (game->platformModel.meshCount > 0) UnloadModel(game->platformModel);
    if (game->rubbleModel.meshCount > 0) UnloadModel(game->rubbleModel);
}
