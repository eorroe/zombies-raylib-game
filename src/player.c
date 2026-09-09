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
    player->model = LoadModelFromMesh(GenMeshCylinder(0.35f, 1.6f, 8));
    player->isMoving = false;
    player->footstepTimer = 0.0f;
}

void PlayerUpdate(Player *player, Camera3D camera, float dt) {
    player->velocity = (Vector3){ 0 };
    player->isMoving = false;
    
    Vector3 cameraForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    cameraForward.y = 0;
    if (Vector3Length(cameraForward) > 0.001f) cameraForward = Vector3Normalize(cameraForward);
    else cameraForward = (Vector3){ 0, 0, 1 };
    Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraForward, (Vector3){ 0, 1, 0 }));
    
    Vector3 moveDir = { 0 };
    if (IsKeyDown(KEY_W)) { moveDir = Vector3Add(moveDir, cameraForward); player->isMoving = true; }
    if (IsKeyDown(KEY_S)) { moveDir = Vector3Subtract(moveDir, cameraForward); player->isMoving = true; }
    if (IsKeyDown(KEY_A)) { moveDir = Vector3Subtract(moveDir, cameraRight); player->isMoving = true; }
    if (IsKeyDown(KEY_D)) { moveDir = Vector3Add(moveDir, cameraRight); player->isMoving = true; }
    
    if (player->isMoving) {
        moveDir = Vector3Normalize(moveDir);
        player->velocity = Vector3Scale(moveDir, PLAYER_SPEED);
        player->position = Vector3Add(player->position, Vector3Scale(player->velocity, dt));
    }
    
    Vector2 mouseDelta = GetMouseDelta();
    player->yaw -= mouseDelta.x * 0.003f;
    player->pitch -= mouseDelta.y * 0.003f;
    player->pitch = Clamp(player->pitch, -PI / 2.0f + 0.1f, PI / 2.0f - 0.1f);
}

void PlayerRender(Player *player, Shader shader) {
    (void)shader;
    (void)player;
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
