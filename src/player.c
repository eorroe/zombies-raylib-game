#include "player.h"
#include "raymath.h"
#include <math.h>

void PlayerInit(Player *player, Vector3 startPos) {
    player->position = startPos;
    player->velocity = (Vector3){ 0 };
    player->health = PLAYER_HEALTH;
    player->maxHealth = PLAYER_HEALTH;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->model = LoadModelFromMesh(GenMeshCylinder(0.5f, 1.8f, 8));
    player->isMoving = false;
    player->footstepTimer = 0.0f;
}

void PlayerUpdate(Player *player, float dt) {
    player->velocity = (Vector3){ 0 };
    player->isMoving = false;
    
    if (IsKeyDown(KEY_W)) { player->velocity.z = 1.0f; player->isMoving = true; }
    if (IsKeyDown(KEY_S)) { player->velocity.z = -1.0f; player->isMoving = true; }
    if (IsKeyDown(KEY_A)) { player->velocity.x = -1.0f; player->isMoving = true; }
    if (IsKeyDown(KEY_D)) { player->velocity.x = 1.0f; player->isMoving = true; }
    
    Vector2 mouseDelta = GetMouseDelta();
    player->yaw -= mouseDelta.x * 0.003f;
    player->pitch -= mouseDelta.y * 0.003f;
    player->pitch = Clamp(player->pitch, -PI / 2.0f + 0.1f, PI / 2.0f - 0.1f);
    
    if (player->isMoving) {
        player->velocity = Vector3Normalize(player->velocity);
        player->velocity.x *= PLAYER_SPEED;
        player->velocity.z *= PLAYER_SPEED;
        
        float cosY = cosf(player->yaw);
        float sinY = sinf(player->yaw);
        Vector3 moved = {
            player->velocity.x * cosY - player->velocity.z * sinY,
            0,
            player->velocity.x * sinY + player->velocity.z * cosY
        };
        player->position = Vector3Add(player->position, Vector3Scale(moved, dt));
    }
}

void PlayerRender(Player *player, Shader shader) {
    DrawModelEx(player->model, player->position, (Vector3){ 0, 1, 0 }, player->yaw * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
}

void PlayerShutdown(Player *player) {
    UnloadModel(player->model);
}

void PlayerTakeDamage(Player *player, float damage) {
    player->health -= damage;
    if (player->health < 0) player->health = 0;
}

Vector3 PlayerGetForward(Player *player) {
    return (Vector3){
        sinf(player->yaw) * cosf(player->pitch),
        -sinf(player->pitch),
        cosf(player->yaw) * cosf(player->pitch)
    };
}

Vector3 PlayerGetRight(Player *player) {
    return (Vector3){
        cosf(player->yaw),
        0,
        -sinf(player->yaw)
    };
}
