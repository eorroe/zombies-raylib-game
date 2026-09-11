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
}

void RendererBegin(Game *game, Camera3D camera) {
    (void)game;
    (void)camera;
    
    int w = 1280;
    int h = 720;
    if (game->screenshotMode == SCREENSHOT_MODE_ZOMBIE || game->screenshotMode == SCREENSHOT_MODE_PLAYER) {
        ClearBackground((Color){ 245, 240, 232, 255 });
    } else {
        DrawRectangleGradientV(0, 0, w, h / 2, (Color){ 120, 160, 220, 255 }, (Color){ 220, 180, 120, 255 });
        DrawRectangleGradientV(0, h / 2, w, h / 2, (Color){ 220, 180, 120, 255 }, (Color){ 180, 120, 80, 255 });
    }
    
    BeginMode3D(camera);
    rlDisableBackfaceCulling();
}

void RendererDrawScene(Game *game) {
    if (!fireLightsInitialized) return;
    
    Vector3 lightPositions[4];
    Vector3 lightColors[4];
    float t = GetTime();
    for (int i = 0; i < 4; i++) {
        lightPositions[i] = fireLightPositions[i];
        float flicker = 1.0f + sinf(t * 8.0f + i * 2.5f) * 0.15f + sinf(t * 13.0f + i * 1.7f) * 0.1f;
        lightColors[i] = (Vector3){ 1.2f * flicker, 0.7f * flicker, 0.4f * flicker };
    }
    int lightCount = 4;
    
    ShaderBeginPBR(&game->shaders);
    ShaderSetFog(&game->shaders, (Vector3){ 0.6f, 0.5f, 0.4f }, 0.001f);
    ShaderSetDirectionalLight(&game->shaders, (Vector3){ 0.5f, 0.8f, 0.3f }, (Vector3){ 1.2f, 0.9f, 0.7f });
    
    for (int i = 0; i < 4; i++) {
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightPos[i], &lightPositions[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCol[i], &lightColors[i], SHADER_UNIFORM_VEC3);
    }
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCount, &lightCount, SHADER_UNIFORM_INT);
    
    SetPBRMaterial(game, game->textures.concrete, game->textures.concreteNormal, 0.1f, 0.7f);
    DrawGrid(50, 1.0f);
    
    SetPBRMaterial(game, game->textures.asphalt, (Texture2D){0}, 0.0f, 0.9f);
    DrawPlane((Vector3){ 0, 0.02f, 0 }, (Vector2){ 50, 50 }, WHITE);
    
    SetPBRMaterial(game, game->textures.container, game->textures.containerNormal, 0.6f, 0.5f);
    DrawModel(game->containerModel, (Vector3){ 0, 0.6f, 0 }, 1.0f, WHITE);
    for (int i = 0; i < 6; i++) {
        float x = -10.0f + i * 3.5f;
        Vector3 containerPos = { x, 0.6f, -6.0f - (i % 2) * 1.0f };
        DrawModel(game->containerModel, containerPos, 1.0f, WHITE);
    }
    for (int i = 0; i < 4; i++) {
        float x = -10.0f + i * 3.5f;
        Vector3 containerPos = { x, 0.6f, -10.0f };
        DrawModel(game->containerModel, containerPos, 1.0f, (Color){ 180, 80, 50, 255 });
    }
    
    SetPBRMaterial(game, game->textures.fence, game->textures.metalNormal, 0.8f, 0.4f);
    for (int i = 0; i < 20; i++) {
        float x = -8.0f + i * 0.8f;
        DrawModel(game->fenceModel, (Vector3){ x, 0.6f, 4.0f }, 1.0f, WHITE);
        DrawModel(game->fenceModel, (Vector3){ x, 0.6f, 4.5f }, 1.0f, WHITE);
    }
    for (int i = 0; i < 20; i++) {
        float x = -8.0f + i * 0.8f;
        float barY = 0.3f + (i % 3) * 0.35f;
        DrawCube((Vector3){ x, barY, 4.0f }, 0.04f, 0.04f, 2.0f, (Color){ 50, 50, 55, 255 });
        DrawCube((Vector3){ x, barY, 4.5f }, 0.04f, 0.04f, 2.0f, (Color){ 50, 50, 55, 255 });
    }
    
    SetPBRMaterial(game, game->textures.concrete, game->textures.concreteNormal, 0.1f, 0.7f);
    DrawModel(game->platformModel, (Vector3){ 8.0f, 0.3f, 3.0f }, 1.0f, WHITE);
    DrawModel(game->platformModel, (Vector3){ 12.0f, 0.3f, 3.0f }, 1.0f, (Color){ 110, 105, 100, 255 });
    DrawCube((Vector3){ 10.0f, 0.55f, 4.2f }, 4.5f, 0.3f, 0.3f, (Color){ 80, 75, 70, 255 });
    
    for (int i = 0; i < 40; i++) {
        float x = -20.0f + (i % 20) * 2.0f + (i / 20) * 0.5f;
        float z = -5.0f + (i % 7) * 1.5f;
        float y = 0.08f + (i % 3) * 0.04f;
        DrawModel(game->rubbleModel, (Vector3){ x, y, z }, 1.0f, (Color){ 90, 85, 80, 255 });
    }
    
    SetPBRMaterial(game, game->textures.concrete, game->textures.concreteNormal, 0.1f, 0.7f);
    for (int i = 0; i < 8; i++) {
        float x = -14.0f + i * 3.0f;
        float z = 14.0f + (i % 2) * 2.0f;
        float h = 1.2f + (i % 4) * 0.8f;
        DrawCube((Vector3){ x, h * 0.5f, z }, 0.5f, h * 0.5f, 1.5f, (Color){ 70, 65, 60, 255 });
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
        float t = GetTime();
        for (int p = 0; p < 25; p++) {
            float ft = t * 3.0f + p * 1.7f + i * 5.0f;
            float fx = fireLightPositions[i].x + sinf(ft * 2.3f) * 0.4f;
            float fy = fireLightPositions[i].y + fmodf(ft * 0.8f, 1.5f) + 0.3f;
            float fz = fireLightPositions[i].z + cosf(ft * 1.9f) * 0.4f;
            float alpha = 0.6f - fmodf(ft * 0.8f, 1.5f) * 0.4f;
            if (alpha < 0.0f) alpha = 0.0f;
            float size = 0.06f + fmodf(ft, 1.0f) * 0.1f;
            DrawSphere((Vector3){ fx, fy, fz }, size, (Color){ 255, 160 + (int)(fmodf(ft, 1.0f) * 80.0f), 40, (unsigned char)(alpha * 255) });
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
    printf("ZOMBIES: count=%d\n", game->zombieCount);
    Camera3D cam = CameraGetCamera(&game->camera);
    ShaderBeginPBR(&game->shaders);
    ShaderSetFog(&game->shaders, (Vector3){ 0.6f, 0.5f, 0.4f }, 0.001f);
    ShaderSetDirectionalLight(&game->shaders, (Vector3){ 0.5f, 0.8f, 0.3f }, (Vector3){ 1.2f, 0.9f, 0.7f });
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
    int w = game->sceneTarget.texture.width;
    int h = game->sceneTarget.texture.height;
    
    DrawRectangle(0, h - 70, w, 70, ColorAlpha(BLACK, 0.6f));
    
    int ammoX = w - 20;
    int ammoY = h - 45;
    const char *ammoText = TextFormat("%d / %d", game->weapon.ammo, MAX_AMMO);
    int ammoW = MeasureText(ammoText, 28);
    DrawText(ammoText, ammoX - ammoW, ammoY, 28, WHITE);
    
    int healthX = 20;
    int healthY = h - 45;
    float healthPct = game->player.health / game->player.maxHealth;
    DrawRectangle(healthX, healthY, 200, 18, (Color){ 60, 60, 60, 200 });
    DrawRectangle(healthX, healthY, (int)(200 * healthPct), 18, (Color){ 180, 40, 40, 255 });
    DrawRectangleLines(healthX, healthY, 200, 18, WHITE);
    DrawText(TextFormat("%.0f", game->player.health), healthX + 205, healthY, 20, WHITE);
    
    if (CameraGetADSBlend(&game->camera) > 0.5f || CameraGetFirstPersonBlend(&game->camera) > 0.5f) {
        int cx = w / 2;
        int cy = h / 2;
        DrawLine(cx - 12, cy, cx - 4, cy, (Color){ 200, 200, 200, 180 });
        DrawLine(cx + 4, cy, cx + 12, cy, (Color){ 200, 200, 200, 180 });
        DrawLine(cx, cy - 12, cx, cy - 4, (Color){ 200, 200, 200, 180 });
        DrawLine(cx, cy + 4, cx, cy + 12, (Color){ 200, 200, 200, 180 });
    }
    
    if (game->weapon.reloading) {
        DrawText("RELOADING", w / 2 - MeasureText("RELOADING", 20) / 2, h - 65, 20, YELLOW);
    }
}

void RendererDrawScope(Game *game) {
    (void)game;
}

void RendererEnd(Game *game) {
    (void)game;
    EndMode3D();
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
