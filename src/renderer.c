#include "renderer.h"
#include "texture.h"
#include "shader.h"
#include "particle.h"
#include "zombie.h"
#include "raymath.h"

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

void RendererInit(Game *game, int screenWidth, int screenHeight) {
    game->sceneTarget = LoadRenderTexture(screenWidth, screenHeight);
    game->postProcessTarget = LoadRenderTexture(screenWidth, screenHeight);
    ShaderInit(&game->shaders, screenWidth, screenHeight);
    ShaderUpdate(&game->shaders, 0.0f, screenWidth, screenHeight);
    
    Mesh crateMesh = GenMeshCube(0.6f, 0.6f, 0.6f);
    game->crateModel = LoadModelFromMesh(crateMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->crateModel, game->textures.concrete);
    }
    
    Mesh barrelMesh = GenMeshCylinder(0.2f, 0.8f, 16);
    game->barrelModel = LoadModelFromMesh(barrelMesh);
    if (game->textures.generated && game->textures.metal.id != 0) {
        SetModelTexture(&game->barrelModel, game->textures.metal);
    }
    
    Mesh wallMesh = GenMeshCube(4.0f, 1.5f, 0.4f);
    game->wallModel = LoadModelFromMesh(wallMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->wallModel, game->textures.concrete);
    }
    
    Mesh buildingMesh = GenMeshCube(2.5f, 3.0f, 2.5f);
    game->buildingModel = LoadModelFromMesh(buildingMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->buildingModel, game->textures.concrete);
    }
    
    Mesh floorMesh = GenMeshPlane(50, 50, 50, 50);
    game->floorModel = LoadModelFromMesh(floorMesh);
    if (game->textures.generated && game->textures.concrete.id != 0) {
        SetModelTexture(&game->floorModel, game->textures.concrete);
    }
    
    Mesh bloodMesh = GenMeshPlane(1.5f, 1.5f, 4, 4);
    game->bloodDecalModel = LoadModelFromMesh(bloodMesh);
    if (game->textures.generated && game->textures.bloodDecal.id != 0) {
        SetModelTexture(&game->bloodDecalModel, game->textures.bloodDecal);
    }
}

void RendererBegin(Game *game, Camera3D camera) {
    (void)game;
    (void)camera;
    BeginTextureMode(game->sceneTarget);
    ClearBackground(BLACK);

    DrawRectangleGradientV(0, 0, game->sceneTarget.texture.width, game->sceneTarget.texture.height / 2, (Color){ 100, 180, 255, 255 }, (Color){ 180, 220, 255, 255 });
    DrawRectangleGradientV(0, game->sceneTarget.texture.height / 2, game->sceneTarget.texture.width, game->sceneTarget.texture.height / 2, (Color){ 180, 220, 255, 255 }, (Color){ 135, 206, 235, 255 });

    BeginMode3D(camera);
}

void RendererDrawScene(Game *game) {
    (void)game;

    if (game->floorModel.meshCount > 0) {
        DrawModel(game->floorModel, (Vector3){ 0, 0, 0 }, 1.0f, WHITE);
    } else {
        DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, (Color){ 90, 90, 95, 255 });
    }

    Vector3 lightDir = Vector3Normalize((Vector3){ 0.5f, 1.0f, 0.3f });
    Vector3 lightPos = Vector3Scale(lightDir, -20.0f);
    lightPos.y = 15.0f;

    BeginShaderMode(game->shaders.pbr);
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCount, &(int){ 1 }, SHADER_UNIFORM_INT);
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightPos[0], &lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocLightCol[0], &(Vector3){ 1.0f, 0.95f, 0.8f }, SHADER_UNIFORM_VEC3);
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocMetallic, &(float){ 0.0f }, SHADER_UNIFORM_FLOAT);
    SetShaderValue(game->shaders.pbr, game->shaders.pbrLocRoughness, &(float){ 0.7f }, SHADER_UNIFORM_FLOAT);
    EndShaderMode();
    
    for (int i = 0; i < 4; i++) {
        float angle = i * PI * 0.5f + PI * 0.25f;
        float radius = 18.0f;
        Vector3 lampPos = {
            cosf(angle) * radius,
            2.0f,
            sinf(angle) * radius
        };
        DrawSphere((Vector3){ lampPos.x, 0.05f, lampPos.z }, 1.5f, (Color){ 255, 240, 200, 40 });
    }

    DrawGrid(50, 1.0f);

    if (game->muzzleFlashTimer > 0) {
        DrawSphere(game->muzzleFlashPos, 0.3f, (Color){ 255, 240, 200, 255 });
    }

    for (int i = 0; i < 8; i++) {
        float angle = i * PI * 0.25f;
        float radius = 12.0f;
        Vector3 cratePos = {
            cosf(angle) * radius,
            0.3f,
            sinf(angle) * radius
        };
        if (game->crateModel.meshCount > 0) {
            DrawModel(game->crateModel, cratePos, 1.0f, WHITE);
        } else {
            DrawCube(cratePos, 0.6f, 0.6f, 0.6f, (Color){ 140, 120, 100, 255 });
            DrawCubeWires(cratePos, 0.6f, 0.6f, 0.6f, (Color){ 80, 70, 60, 255 });
        }
    }

    for (int i = 0; i < 4; i++) {
        float angle = i * PI * 0.5f + 0.3f;
        float radius = 8.0f;
        Vector3 barrelPos = {
            cosf(angle) * radius,
            0.4f,
            sinf(angle) * radius
        };
        if (game->barrelModel.meshCount > 0) {
            DrawModel(game->barrelModel, barrelPos, 1.0f, WHITE);
        } else {
            DrawCylinder(barrelPos, 0.2f, 0.2f, 0.8f, 12, (Color){ 160, 140, 110, 255 });
            DrawCylinderWires(barrelPos, 0.2f, 0.2f, 0.8f, 12, (Color){ 100, 90, 75, 255 });
        }
    }

    for (int i = 0; i < 5; i++) {
        float x = -10.0f + i * 5.0f;
        Vector3 wallPos = { x, 0.75f, -10.0f };
        if (game->wallModel.meshCount > 0) {
            DrawModel(game->wallModel, wallPos, 1.0f, WHITE);
        } else {
            DrawCube(wallPos, 4.0f, 1.5f, 0.4f, (Color){ 150, 145, 140, 255 });
            DrawCubeWires(wallPos, 4.0f, 1.5f, 0.4f, (Color){ 90, 85, 80, 255 });
        }
    }

    for (int i = 0; i < 6; i++) {
        float x = -8.0f + i * 3.5f;
        float z = 8.0f;
        float h = 1.5f + (i % 3) * 1.0f;
        Vector3 bldPos = { x, h * 0.5f, z };
        if (game->buildingModel.meshCount > 0) {
            DrawModelEx(game->buildingModel, bldPos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, h / 3.0f, 1 }, WHITE);
        } else {
            Color bldColor = (Color){ 130, 130, 140, 255 };
            if (i % 3 == 1) bldColor = (Color){ 150, 140, 130, 255 };
            else if (i % 3 == 2) bldColor = (Color){ 120, 130, 150, 255 };
            DrawCube(bldPos, 2.5f, h, 2.5f, bldColor);
            DrawCubeWires(bldPos, 2.5f, h, 2.5f, (Color){ 80, 80, 90, 255 });
        }

        for (int w = 0; w < 3; w++) {
            float wx = x - 0.6f + w * 0.6f;
            float wy = h * 0.5f + 0.3f;
            DrawCube((Vector3){ wx, wy, z + 1.26f }, 0.3f, 0.3f, 0.05f, (Color){ 60, 60, 70, 255 });
        }
    }

    for (int i = 0; i < 4; i++) {
        float angle = i * PI * 0.5f + PI * 0.25f;
        float radius = 18.0f;
        Vector3 lampPos = {
            cosf(angle) * radius,
            2.0f,
            sinf(angle) * radius
        };
        DrawCylinder(lampPos, 0.1f, 0.1f, 2.0f, 8, (Color){ 60, 60, 65, 255 });
        DrawSphere((Vector3){ lampPos.x, lampPos.y + 1.0f, lampPos.z }, 0.3f, (Color){ 255, 240, 200, 255 });
    }
}

void RendererDrawBloodDecals(Game *game) {
    if (game->bloodDecalCount <= 0) return;
    for (int i = 0; i < game->bloodDecalCount; i++) {
        Vector3 pos = game->bloodDecals[i];
        pos.y = 0.02f;
        if (game->bloodDecalModel.meshCount > 0) {
            DrawModelEx(game->bloodDecalModel, pos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 2.0f, 2.0f, 2.0f }, WHITE);
        } else {
            DrawPlane(pos, (Vector2){ 3.0f, 3.0f }, (Color){ 200, 0, 0, 200 });
        }
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
        if (z->type != ZOMBIE_TYPE_IMAGE_HEAD) continue;
        if (game->zombieHeadTextureCount <= 0) continue;
        if (z->textureIndex < 0 || z->textureIndex >= game->zombieHeadTextureCount) continue;
        Texture2D tex = game->zombieHeadTextures[z->textureIndex];
        if (tex.id == 0) continue;
        
        float bob = sinf(z->animTime) * 0.05f;
        float headY = z->position.y + bob + LEG_UPPER_LEN + LEG_LOWER_LEN + TORSO_HEIGHT + HEAD_RADIUS * 0.9f;
        Vector3 headPos = (Vector3){ z->position.x, headY, z->position.z };
        Vector2 screenPos = GetWorldToScreen(headPos, cam);
        float dist = Vector3Length(Vector3Subtract(cam.position, headPos));
        float size = 120.0f * 8.0f / dist;
        Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
        Rectangle dst = { screenPos.x - size / 2, screenPos.y - size / 2, size, size };
        DrawTexturePro(tex, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    }
}

void RendererDrawPlayer(Player *player, Shader shader) {
    (void)shader;
    PlayerRender(player, shader);
}

void RendererDrawParticles(Particle *particles, int count) {
    ParticleSystemRender(particles, count);
    
    for (int i = 0; i < 80; i++) {
        float t = GetTime() * 0.15f + i * 2.17f;
        float x = sinf(t * 1.3f) * 18.0f;
        float y = 0.3f + fmodf(t * 0.4f, 4.0f);
        float z = cosf(t * 0.9f) * 18.0f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.4f + sinf(t + i) * 0.2f;
        DrawSphere(dustPos, 0.12f, (Color){ 255, 255, 240, (unsigned char)(alpha * 255) });
    }
}

void RendererDrawHUD(Game *game) {
    DrawRectangle(0, 0, game->sceneTarget.texture.width, 60, ColorAlpha(BLACK, 0.5f));
    DrawText(TextFormat("Score: %d", game->score), 20, 20, 30, WHITE);
    DrawText(TextFormat("Health: %.0f", game->player.health), 200, 20, 30, WHITE);
    DrawText(TextFormat("Ammo: %d", game->weapon.ammo), 400, 20, 30, WHITE);
    if (game->weapon.reloading) {
        DrawText("RELOADING...", 600, 20, 30, YELLOW);
    }
    
    int cx = game->sceneTarget.texture.width / 2;
    int cy = game->sceneTarget.texture.height / 2;
    DrawCircle(cx, cy, 2, WHITE);
    DrawLine(cx - 10, cy, cx - 5, cy, WHITE);
    DrawLine(cx + 5, cy, cx + 10, cy, WHITE);
    DrawLine(cx, cy - 10, cx, cy - 5, WHITE);
    DrawLine(cx, cy + 5, cx, cy + 10, WHITE);
}

void RendererDrawScope(Game *game) {
    if (!game->scopeActive) return;
    BeginShaderMode(game->shaders.scope);
    DrawTexture(game->postProcessTarget.texture, 0, 0, WHITE);
    EndShaderMode();
}

void RendererEnd(Game *game) {
    EndMode3D();
    RendererDrawZombieHeads(game);
    RendererDrawHUD(game);
    EndTextureMode();
    
    BeginTextureMode(game->postProcessTarget);
    DrawTexture(game->sceneTarget.texture, 0, 0, WHITE);
    EndTextureMode();
    
    if (game->scopeActive) {
        BeginShaderMode(game->shaders.scope);
        DrawTexture(game->postProcessTarget.texture, 0, 0, WHITE);
        EndShaderMode();
    } else {
        DrawTexture(game->postProcessTarget.texture, 0, 0, WHITE);
    }
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
