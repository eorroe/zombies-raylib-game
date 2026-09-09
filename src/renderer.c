#include "renderer.h"
#include "texture.h"
#include "shader.h"
#include "particle.h"
#include "zombie.h"
#include "raymath.h"

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
    
    Mesh barrelMesh = GenMeshCylinder(0.2f, 0.8f, 16);
    game->barrelModel = LoadModelFromMesh(barrelMesh);
    if (game->textures.generated && game->textures.metal.id != 0) {
        SetModelTexture(&game->barrelModel, game->textures.metal, game->textures.metalNormal);
    }
    
    Mesh wallMesh = GenMeshCube(4.5f, 1.5f, 0.4f);
    game->wallModel = LoadModelFromMesh(wallMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->wallModel, game->textures.concrete, game->textures.concreteNormal);
    }
    
    Mesh buildingMesh = GenMeshCube(2.5f, 3.0f, 2.5f);
    game->buildingModel = LoadModelFromMesh(buildingMesh);
    if (game->textures.generated && game->textures.brick.id != 0) {
        SetModelTexture(&game->buildingModel, game->textures.brick, (Texture2D){0});
    }
    
    Mesh floorMesh = GenMeshPlane(50, 50, 50, 50);
    game->floorModel = LoadModelFromMesh(floorMesh);
    if (game->textures.generated && game->textures.asphalt.id != 0) {
        SetModelTexture(&game->floorModel, game->textures.asphalt, (Texture2D){0});
    }
    
    Mesh bloodMesh = GenMeshPlane(1.5f, 1.5f, 4, 4);
    game->bloodDecalModel = LoadModelFromMesh(bloodMesh);
    if (game->textures.generated && game->textures.bloodDecal.id != 0) {
        SetModelTexture(&game->bloodDecalModel, game->textures.bloodDecal, (Texture2D){0});
    }
    
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
}

void RendererBegin(Game *game, Camera3D camera) {
    (void)game;
    (void)camera;
    BeginTextureMode(game->sceneTarget);
    
    int w = game->sceneTarget.texture.width;
    int h = game->sceneTarget.texture.height;
    DrawRectangleGradientV(0, 0, w, h / 2, (Color){ 45, 30, 35, 255 }, (Color){ 80, 50, 45, 255 });
    DrawRectangleGradientV(0, h / 2, w, h / 2, (Color){ 80, 50, 45, 255 }, (Color){ 40, 22, 18, 255 });
    
    BeginMode3D(camera);
}

void RendererDrawScene(Game *game) {
    if (!fireLightsInitialized) return;
    
    Vector3 lightPositions[4];
    Vector3 lightColors[4];
    float t = GetTime();
    for (int i = 0; i < 4; i++) {
        lightPositions[i] = fireLightPositions[i];
        float flicker = 1.0f + sinf(t * 8.0f + i * 2.5f) * 0.15f + sinf(t * 13.0f + i * 1.7f) * 0.1f;
        lightColors[i] = (Vector3){ 1.0f * flicker, (0.45f + i * 0.05f) * flicker, (0.08f + i * 0.04f) * flicker };
    }
    int lightCount = 4;
    
    for (int i = 0; i < 4; i++) {
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightPos[i], &lightPositions[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCol[i], &lightColors[i], SHADER_UNIFORM_VEC3);
    }
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCount, &lightCount, SHADER_UNIFORM_INT);
    
    ShaderBeginPBR(&game->shaders);
    ShaderSetFog(&game->shaders, (Vector3){ 0.08, 0.05, 0.12 }, 0.035f);
    
    SetPBRMaterial(game, game->textures.asphalt, (Texture2D){0}, 0.0f, 0.9f);
    if (game->textures.generated && game->textures.asphalt.id != 0) {
        DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, WHITE);
    } else {
        DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, (Color){ 60, 58, 55, 255 });
    }
    
    SetPBRMaterial(game, game->textures.wood, game->textures.crateNormal, 0.0f, 0.8f);
    for (int i = 0; i < 16; i++) {
        float angle = i * PI * 0.25f;
        float radius = 5.0f + (i % 3) * 3.0f;
        Vector3 cratePos = {
            cosf(angle) * radius,
            0.3f,
            sinf(angle) * radius
        };
        DrawModel(game->crateModel, cratePos, 1.0f, WHITE);
    }
    
    SetPBRMaterial(game, game->textures.metal, game->textures.metalNormal, 0.9f, 0.35f);
    for (int i = 0; i < 10; i++) {
        float angle = i * PI * 0.5f + 0.3f;
        float radius = 3.0f + (i % 2) * 3.0f;
        Vector3 barrelPos = {
            cosf(angle) * radius,
            0.4f,
            sinf(angle) * radius
        };
        DrawModel(game->barrelModel, barrelPos, 1.0f, WHITE);
    }
    
    SetPBRMaterial(game, game->textures.metal, game->textures.metalNormal, 0.7f, 0.5f);
    for (int i = 0; i < 5; i++) {
        float angle = i * PI * 1.2f + 0.7f;
        float radius = 10.0f + (i % 2) * 5.0f;
        Vector3 carPos = {
            cosf(angle) * radius,
            0.6f,
            sinf(angle) * radius
        };
        DrawCube(carPos, 2.0f, 1.0f, 4.0f, (Color){ 40, 35, 30, 255 });
        DrawCube((Vector3){ carPos.x, carPos.y + 0.7f, carPos.z - 1.2f }, 1.6f, 0.6f, 1.8f, (Color){ 30, 30, 35, 255 });
    }
    
    SetPBRMaterial(game, game->textures.wood, game->textures.crateNormal, 0.0f, 0.9f);
    for (int i = 0; i < 8; i++) {
        float angle = i * PI * 0.75f + 1.5f;
        float radius = 8.0f + (i % 3) * 4.0f;
        Vector3 debrisPos = {
            cosf(angle) * radius,
            0.15f,
            sinf(angle) * radius
        };
        DrawCube(debrisPos, 0.4f + (i % 2) * 0.3f, 0.15f, 0.4f + (i % 3) * 0.2f, (Color){ 60, 45, 30, 255 });
    }
    
    SetPBRMaterial(game, game->textures.concrete, game->textures.concreteNormal, 0.1f, 0.7f);
    for (int i = 0; i < 6; i++) {
        float angle = i * PI * 0.5f + 0.2f;
        float radius = 12.0f + (i % 2) * 6.0f;
        Vector3 bodyPos = {
            cosf(angle) * radius,
            0.1f,
            sinf(angle) * radius
        };
        DrawCube(bodyPos, 0.5f, 0.15f, 1.8f, (Color){ 35, 30, 28, 255 });
        DrawSphere((Vector3){ bodyPos.x, bodyPos.y + 0.25f, bodyPos.z - 0.6f }, 0.2f, (Color){ 200, 170, 150, 255 });
    }
    
    for (int i = 0; i < 4; i++) {
        float angle = i * PI * 0.5f + PI * 0.25f;
        float radius = 16.0f;
        Vector3 tirePos = {
            cosf(angle) * radius,
            0.25f,
            sinf(angle) * radius
        };
        DrawCylinder(tirePos, 0.35f, 0.35f, 0.12f, 12, (Color){ 20, 20, 20, 255 });
    }
    
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < 4; i++) {
        float t = GetTime();
        for (int p = 0; p < 25; p++) {
            float ft = t * 3.0f + p * 1.7f + i * 5.0f;
            float fx = fireLightPositions[i].x + sinf(ft * 2.3f) * 0.4f;
            float fy = fireLightPositions[i].y + fmodf(ft * 0.8f, 1.5f) + 0.3f;
            float fz = fireLightPositions[i].z + cosf(ft * 1.9f) * 0.4f;
            float alpha = 0.6f - fmodf(ft * 0.8f, 1.5f) * 0.4f;
            if (alpha < 0.0f) alpha = 0.0f;
            float size = 0.08f + fmodf(ft, 1.0f) * 0.12f;
            DrawSphere((Vector3){ fx, fy, fz }, size, (Color){ 255, 160 + (int)(fmodf(ft, 1.0f) * 80.0f), 40, (unsigned char)(alpha * 255) });
        }
    }
    EndBlendMode();
    
    {
        Vector3 wallPos = { 0, 0.75f, -10.0f };
        DrawModel(game->wallModel, wallPos, 1.0f, WHITE);
    }
    
    SetPBRMaterial(game, game->textures.brick, (Texture2D){0}, 0.0f, 0.85f);
    for (int i = 0; i < 12; i++) {
        float x = -12.0f + i * 4.0f;
        float z = 10.0f;
        float h = 1.5f + (i % 3) * 1.0f;
        Vector3 bldPos = { x, h * 0.5f, z };
        DrawModelEx(game->buildingModel, bldPos, (Vector3){0,1,0}, 0.0f, (Vector3){1.4f, h / 3.0f, 1.4f}, WHITE);
        
        for (int w = 0; w < 5; w++) {
            float wx = x - 1.0f + w * 1.0f;
            float wy = h * 0.5f + 0.3f;
            DrawCube((Vector3){ wx, wy, z + 1.76f }, 0.5f, 0.5f, 0.05f, (Color){ 12, 10, 8, 255 });
        }
        
        if (i % 4 == 0) {
            float cx = x + 0.3f;
            float cz = z - 0.5f;
            DrawCube((Vector3){ cx, h * 0.5f, cz }, 0.08f, h * 0.6f, 0.08f, (Color){ 35, 30, 25, 255 });
            DrawCube((Vector3){ cx + 0.3f, h * 0.3f, cz + 0.2f }, 0.08f, h * 0.3f, 0.08f, (Color){ 35, 30, 25, 255 });
        }
    }
    
    for (int i = 0; i < 3; i++) {
        float z = -6.0f + i * 6.0f;
        float xStart = -18.0f;
        float xEnd = 18.0f;
        for (float x = xStart; x < xEnd; x += 1.5f) {
            float postH = 1.0f + (i % 2) * 0.3f;
            DrawCube((Vector3){ x, postH * 0.5f, z }, 0.08f, postH, 0.08f, (Color){ 40, 38, 35, 255 });
            DrawCube((Vector3){ x, postH * 0.5f, z + 0.15f }, 0.06f, postH * 0.02f, 0.06f, (Color){ 25, 25, 25, 255 });
        }
    }
    
    ShaderEnd(&game->shaders);
    
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < 4; i++) {
        Vector3 beamPos = { fireLightPositions[i].x, fireLightPositions[i].y + 1.5f, fireLightPositions[i].z };
        DrawCylinder(beamPos, 0.1f, 2.5f, 3.0f, 8, (Color){ 255, 150, 50, 18 });
        DrawSphere((Vector3){ fireLightPositions[i].x, 0.05f, fireLightPositions[i].z }, 2.2f, (Color){ 255, 180, 80, 35 });
        DrawSphere((Vector3){ fireLightPositions[i].x, 0.05f, fireLightPositions[i].z }, 0.15f, (Color){ 255, 240, 200, 255 });
    }
    EndBlendMode();
    
    for (int i = 0; i < 180; i++) {
        float t = GetTime() * 0.5f + i * 3.31f;
        int fireIdx = i % 4;
        Vector3 basePos = fireLightPositions[fireIdx];
        float x = basePos.x + sinf(t * 1.7f + fireIdx * 2.0f) * 1.2f;
        float y = basePos.y + fmodf(t * 0.4f, 2.5f) + 0.2f;
        float z = basePos.z + cosf(t * 1.3f + fireIdx * 2.0f) * 1.2f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.25f + sinf(t + i) * 0.15f;
        DrawSphere(dustPos, 0.12f, (Color){ 255, 210, 160, (unsigned char)(alpha * 255) });
    }
    
    for (int i = 0; i < 250; i++) {
        float t = GetTime() * 0.25f + i * 2.17f;
        float x = sinf(t * 1.1f) * 26.0f;
        float y = 0.4f + fmodf(t * 0.5f, 5.0f);
        float z = cosf(t * 0.8f) * 26.0f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.4f + sinf(t + i) * 0.25f;
        DrawSphere(dustPos, 0.18f, (Color){ 255, 255, 240, (unsigned char)(alpha * 255) });
    }
}

void RendererDrawBloodDecals(Game *game) {
    if (game->bloodDecalCount <= 0) return;
    for (int i = 0; i < game->bloodDecalCount; i++) {
        Vector3 pos = game->bloodDecals[i];
        pos.y = 0.03f;
        DrawPlane(pos, (Vector2){ 5.0f, 5.0f }, (Color){ 180, 0, 0, 200 });
    }
}

void RendererDrawZombies(Game *game, Shader shader) {
    (void)shader;
    Camera3D cam = CameraGetCamera(&game->camera);
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieRender(&game->zombies[i], cam, game->zombieHeadTextures, game->zombieHeadTextureCount, shader);
    }
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
                DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 120, 0, 0, 255 });
                DrawRectangle(barX, barY, barWidth * healthPct, barHeight, RED);
            }
        } else {
            Vector2 screenPos = GetWorldToScreen(headPos, cam);
            float barWidth = 40.0f;
            float barHeight = 4.0f;
            float healthPct = (z->maxHealth > 0.0f) ? (z->health / z->maxHealth) : 0.0f;
            DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth, barHeight, (Color){ 120, 0, 0, 255 });
            DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth * healthPct, barHeight, RED);
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
        }
    }
}

void RendererDrawHUD(Game *game) {
    DrawRectangle(0, 0, game->sceneTarget.texture.width, 60, ColorAlpha(BLACK, 0.5f));
    int x = 20;
    int gap = 10;
    int y = 20;
    int fontSize = 30;
    Color color = WHITE;

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
        DrawText("RELOADING...", x, y, fontSize, YELLOW);
    }
    
    if (CameraGetFirstPersonBlend(&game->camera) > 0.5f) {
        int cx = game->sceneTarget.texture.width / 2;
        int cy = game->sceneTarget.texture.height / 2;
        DrawLine(cx - 10, cy, cx + 10, cy, RED);
        DrawLine(cx, cy - 10, cx, cy + 10, RED);
    }
}

void RendererDrawScope(Game *game) {
    (void)game;
}

void RendererEnd(Game *game) {
    (void)game;
    EndMode3D();
    EndTextureMode();

    BeginShaderMode(game->shaders.postProcess);
    DrawTexturePro(game->sceneTarget.texture, (Rectangle){ 0, 0, (float)game->sceneTarget.texture.width, -(float)game->sceneTarget.texture.height }, (Rectangle){ 0, 0, (float)game->sceneTarget.texture.width, (float)game->sceneTarget.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndShaderMode();
}

void RendererShutdown(Game *game) {
    UnloadRenderTexture(game->sceneTarget);
    UnloadRenderTexture(game->postProcessTarget);
    if (game->crateModel.meshCount > 0) UnloadModel(game->crateModel);
    if (game->barrelModel.meshCount > 0) UnloadModel(game->barrelModel);
    if (game->wallModel.meshCount > 0) UnloadModel(game->wallModel);
    if (game->buildingModel.meshCount > 0) UnloadModel(game->buildingModel);
    if (game->floorModel.meshCount > 0) UnloadModel(game->floorModel);
    if (game->bloodDecalModel.meshCount > 0) UnloadModel(game->bloodDecalModel);
}
