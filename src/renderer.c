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
    
    Mesh wallMesh = GenMeshCube(4.5f, 1.5f, 0.4f);
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
    ClearBackground((Color){ 245, 240, 232, 255 });
    BeginMode3D(camera);
}

void RendererDrawScene(Game *game) {
    (void)game;

    DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, (Color){ 210, 205, 195, 255 });
    DrawGrid(50, 1.0f);

    for (int i = 0; i < 16; i++) {
        float angle = i * PI * 0.25f;
        float radius = 5.0f + (i % 3) * 3.0f;
        Vector3 cratePos = {
            cosf(angle) * radius,
            0.3f,
            sinf(angle) * radius
        };
        DrawCube(cratePos, 1.0f, 1.0f, 1.0f, (Color){ 200, 180, 160, 255 });
        DrawCubeWires(cratePos, 1.0f, 1.0f, 1.0f, (Color){ 40, 40, 45, 255 });
    }

    for (int i = 0; i < 10; i++) {
        float angle = i * PI * 0.5f + 0.3f;
        float radius = 3.0f + (i % 2) * 3.0f;
        Vector3 barrelPos = {
            cosf(angle) * radius,
            0.4f,
            sinf(angle) * radius
        };
        DrawCylinder(barrelPos, 0.3f, 0.3f, 1.2f, 12, (Color){ 190, 175, 155, 255 });
        DrawCylinderWires(barrelPos, 0.3f, 0.3f, 1.2f, 12, (Color){ 40, 40, 45, 255 });
    }

    for (int i = 0; i < 1; i++) {
        Vector3 wallPos = { 0, 0.75f, -10.0f };
        DrawCube(wallPos, 38.0f, 1.5f, 0.6f, (Color){ 190, 185, 180, 255 });
        DrawCubeWires(wallPos, 38.0f, 1.5f, 0.6f, (Color){ 40, 40, 45, 255 });
    }

    for (int i = 0; i < 12; i++) {
        float x = -12.0f + i * 4.0f;
        float z = 10.0f;
        float h = 1.5f + (i % 3) * 1.0f;
        Vector3 bldPos = { x, h * 0.5f, z };
        Color bldColor = (Color){ 175, 170, 165, 255 };
        if (i % 3 == 1) bldColor = (Color){ 185, 175, 165, 255 };
        else if (i % 3 == 2) bldColor = (Color){ 170, 175, 185, 255 };
        DrawCube(bldPos, 3.5f, h, 3.5f, bldColor);
        DrawCubeWires(bldPos, 3.5f, h, 3.5f, (Color){ 40, 40, 45, 255 });

        for (int w = 0; w < 5; w++) {
            float wx = x - 1.0f + w * 1.0f;
            float wy = h * 0.5f + 0.3f;
            DrawCube((Vector3){ wx, wy, z + 1.76f }, 0.5f, 0.5f, 0.05f, (Color){ 140, 50, 50, 255 });
            DrawCubeWires((Vector3){ wx, wy, z + 1.76f }, 0.5f, 0.5f, 0.05f, (Color){ 40, 40, 45, 255 });
        }
    }

    for (int i = 0; i < 8; i++) {
        float angle = i * PI * 0.5f + PI * 0.25f;
        float radius = 18.0f;
        Vector3 lampPos = {
            cosf(angle) * radius,
            2.0f,
            sinf(angle) * radius
        };
        DrawCylinder(lampPos, 0.2f, 0.2f, 3.0f, 8, (Color){ 110, 108, 105, 255 });
        DrawCylinderWires(lampPos, 0.2f, 0.2f, 3.0f, 8, (Color){ 40, 40, 45, 255 });
        DrawSphereWires((Vector3){ lampPos.x, lampPos.y + 1.5f, lampPos.z }, 0.5f, 8, 8, (Color){ 40, 40, 45, 255 });
        DrawSphere((Vector3){ lampPos.x, 0.05f, lampPos.z }, 3.0f, (Color){ 240, 200, 100, 60 });
        DrawSphereWires((Vector3){ lampPos.x, 0.05f, lampPos.z }, 3.0f, 8, 8, (Color){ 40, 40, 45, 60 });
    }
}

void RendererDrawBloodDecals(Game *game) {
    if (game->bloodDecalCount <= 0) return;
    for (int i = 0; i < game->bloodDecalCount; i++) {
        Vector3 pos = game->bloodDecals[i];
        pos.y = 0.03f;
        DrawPlane(pos, (Vector2){ 5.0f, 5.0f }, (Color){ 120, 20, 20, 200 });
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
                DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 60, 40, 40, 255 });
                DrawRectangle(barX, barY, barWidth * healthPct, barHeight, (Color){ 120, 50, 50, 255 });
            }
        } else {
            Vector2 screenPos = GetWorldToScreen(headPos, cam);
            float barWidth = 40.0f;
            float barHeight = 4.0f;
            float healthPct = (z->maxHealth > 0.0f) ? (z->health / z->maxHealth) : 0.0f;
            DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth, barHeight, (Color){ 60, 40, 40, 255 });
            DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth * healthPct, barHeight, (Color){ 120, 50, 50, 255 });
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
            DrawSphereWires(particles[i].position, particles[i].size * 4.0f, 8, 8, (Color){ 40, 40, 45, 180 });
        }
    }
    
    for (int i = 0; i < 500; i++) {
        float t = GetTime() * 0.4f + i * 2.17f;
        float x = sinf(t * 1.3f) * 28.0f;
        float y = 0.8f + fmodf(t * 0.8f, 8.0f);
        float z = cosf(t * 0.9f) * 28.0f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.8f + sinf(t + i) * 0.2f;
        DrawSphere(dustPos, 0.3f, (Color){ 200, 190, 170, (unsigned char)(alpha * 255) });
        DrawSphereWires(dustPos, 0.3f, 8, 8, (Color){ 40, 40, 45, (unsigned char)(alpha * 180) });
    }
}

void RendererDrawHUD(Game *game) {
    DrawRectangle(0, 0, game->sceneTarget.texture.width, 60, (Color){ 245, 240, 232, 200 });
    DrawRectangleLines(0, 0, game->sceneTarget.texture.width, 60, (Color){ 40, 40, 45, 255 });
    int x = 20;
    int gap = 10;
    int y = 20;
    int fontSize = 30;
    Color color = (Color){ 35, 35, 40, 255 };

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
        DrawText("RELOADING...", x, y, fontSize, (Color){ 140, 50, 50, 255 });
    }
    
    if (CameraGetFirstPersonBlend(&game->camera) > 0.5f) {
        int cx = game->sceneTarget.texture.width / 2;
        int cy = game->sceneTarget.texture.height / 2;
        DrawLine(cx - 10, cy, cx + 10, cy, (Color){ 40, 40, 45, 255 });
        DrawLine(cx, cy - 10, cx, cy + 10, (Color){ 40, 40, 45, 255 });
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
