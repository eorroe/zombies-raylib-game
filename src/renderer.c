#include "renderer.h"
#include "texture.h"
#include "shader.h"
#include "particle.h"
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

void RendererDrawZombies(Game *game, Shader shader) {
    (void)shader;
    for (int i = 0; i < game->zombieCount; i++) {
        ZombieRender(&game->zombies[i], shader);
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
