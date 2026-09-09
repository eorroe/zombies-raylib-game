#include "renderer.h"
#include "texture.h"
#include "shader.h"
#include "particle.h"
#include "zombie.h"
#include "raymath.h"

void RendererInit(Game *game, int screenWidth, int screenHeight) {
    game->sceneTarget = LoadRenderTexture(screenWidth, screenHeight);
    game->postProcessTarget = LoadRenderTexture(screenWidth, screenHeight);
    ShaderInit(&game->shaders, screenWidth, screenHeight);
    ShaderUpdate(&game->shaders, 0.0f, screenWidth, screenHeight);
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

    Color floorColor = (Color){ 50, 50, 55, 255 };
    if (game->textures.generated && game->textures.concrete.id != 0) {
        floorColor = (Color){ 70, 70, 75, 255 };
    }
    DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, floorColor);

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
        DrawCube(cratePos, 0.6f, 0.6f, 0.6f, (Color){ 80, 70, 60, 255 });
        DrawCubeWires(cratePos, 0.6f, 0.6f, 0.6f, (Color){ 50, 45, 40, 255 });
    }

    for (int i = 0; i < 4; i++) {
        float angle = i * PI * 0.5f + 0.3f;
        float radius = 8.0f;
        Vector3 barrelPos = {
            cosf(angle) * radius,
            0.4f,
            sinf(angle) * radius
        };
        DrawCylinder(barrelPos, 0.2f, 0.2f, 0.8f, 12, (Color){ 100, 90, 70, 255 });
        DrawCylinderWires(barrelPos, 0.2f, 0.2f, 0.8f, 12, (Color){ 60, 55, 45, 255 });
    }

    for (int i = 0; i < 5; i++) {
        float x = -10.0f + i * 5.0f;
        Vector3 wallPos = { x, 0.75f, -10.0f };
        DrawCube(wallPos, 4.0f, 1.5f, 0.4f, (Color){ 90, 85, 80, 255 });
        DrawCubeWires(wallPos, 4.0f, 1.5f, 0.4f, (Color){ 55, 50, 45, 255 });
    }

    for (int i = 0; i < 6; i++) {
        float x = -8.0f + i * 3.5f;
        float z = 8.0f;
        float h = 1.5f + (i % 3) * 1.0f;
        Vector3 bldPos = { x, h * 0.5f, z };
        Color bldColor = (Color){ 75, 75, 80, 255 };
        if (i % 3 == 1) bldColor = (Color){ 85, 80, 75, 255 };
        else if (i % 3 == 2) bldColor = (Color){ 70, 75, 85, 255 };
        DrawCube(bldPos, 2.5f, h, 2.5f, bldColor);
        DrawCubeWires(bldPos, 2.5f, h, 2.5f, (Color){ 45, 45, 50, 255 });

        for (int w = 0; w < 3; w++) {
            float wx = x - 0.6f + w * 0.6f;
            float wy = h * 0.5f + 0.3f;
            DrawCube((Vector3){ wx, wy, z + 1.26f }, 0.3f, 0.3f, 0.05f, (Color){ 40, 40, 50, 255 });
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
        DrawPlane(pos, (Vector2){ 1.5f, 1.5f }, (Color){ 140, 0, 0, 160 });
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
    
    for (int i = 0; i < 50; i++) {
        float t = GetTime() * 0.1f + i * 1.37f;
        float x = sinf(t * 1.3f) * 15.0f;
        float y = 0.2f + fmodf(t * 0.5f, 3.0f);
        float z = cosf(t * 0.9f) * 15.0f;
        Vector3 dustPos = { x, y, z };
        float alpha = 0.3f + sinf(t + i) * 0.15f;
        DrawSphere(dustPos, 0.08f, (Color){ 220, 220, 210, (unsigned char)(alpha * 255) });
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
}
