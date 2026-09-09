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
    ClearBackground(SKYBLUE);
    BeginMode3D(camera);
}

void RendererDrawScene(Game *game) {
    (void)game;
    DrawGrid(50, 1.0f);
    DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, (Color){ 60, 60, 60, 255 });
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
}

void RendererDrawHUD(Game *game) {
    DrawRectangle(0, 0, game->sceneTarget.texture.width, 60, ColorAlpha(BLACK, 0.5f));
    DrawText(TextFormat("Score: %d", game->score), 20, 20, 30, WHITE);
    DrawText(TextFormat("Health: %.0f", game->player.health), 200, 20, 30, WHITE);
    DrawText(TextFormat("Ammo: %d", game->weapon.ammo), 400, 20, 30, WHITE);
    if (game->weapon.reloading) {
        DrawText("RELOADING...", 600, 20, 30, YELLOW);
    }
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
