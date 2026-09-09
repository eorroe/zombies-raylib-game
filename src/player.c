#include "player.h"
#include "raymath.h"
#include "texture.h"
#include "input.h"
#include <stdlib.h>
#include <math.h>

static Model CreateLimb(float radius, float length, int slices) {
    Mesh m = GenMeshCylinder(radius, length, slices);
    Model model = LoadModelFromMesh(m);
    return model;
}

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

void PlayerInit(Player *player, Vector3 startPos) {
    player->position = startPos;
    player->velocity = (Vector3){ 0 };
    player->health = PLAYER_HEALTH;
    player->maxHealth = PLAYER_HEALTH;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->isMoving = false;
    player->footstepTimer = 0.0f;
    player->animTime = 0.0f;

    Mesh bodyMesh = GenMeshCylinder(0.4f, 1.2f, 8);
    player->bodyModel = LoadModelFromMesh(bodyMesh);

    Mesh headMesh = GenMeshSphere(0.25f, 12, 12);
    player->headModel = LoadModelFromMesh(headMesh);

    player->leftArmModel = CreateLimb(0.08f, 0.7f, 8);
    player->rightArmModel = CreateLimb(0.08f, 0.7f, 8);
    player->leftLegModel = CreateLimb(0.1f, 0.9f, 8);
    player->rightLegModel = CreateLimb(0.1f, 0.9f, 8);

    Image uniformImg = GenImageColor(256, 256, (Color){ 130, 135, 160, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 110 + rand() % 70;
        ImageDrawPixel(&uniformImg, x, y, (Color){ shade, shade + 10, shade + 25, 255 });
    }
    Texture2D uniformTex = LoadTextureFromImage(uniformImg);
    UnloadImage(uniformImg);

    Image skinImg = GenImageColor(256, 256, (Color){ 180, 220, 140, 255 });
    for (int i = 0; i < 1500; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 140 + rand() % 100;
        ImageDrawPixel(&skinImg, x, y, (Color){ shade, shade + 50, shade - 5, 255 });
    }
    Texture2D skinTex = LoadTextureFromImage(skinImg);
    UnloadImage(skinImg);

    SetModelTexture(&player->bodyModel, uniformTex);
    SetModelTexture(&player->headModel, skinTex);
    SetModelTexture(&player->leftArmModel, uniformTex);
    SetModelTexture(&player->rightArmModel, uniformTex);
    SetModelTexture(&player->leftLegModel, uniformTex);
    SetModelTexture(&player->rightLegModel, uniformTex);

    player->uniformTex = uniformTex;
    player->skinTex = skinTex;
}

void PlayerUpdate(Player *player, InputState *input, float dt) {
    player->velocity = (Vector3){ 0 };
    player->isMoving = false;
    
    Vector3 cameraForward = (Vector3){ sinf(player->yaw), 0, cosf(player->yaw) };
    Vector3 cameraRight = (Vector3){ cosf(player->yaw), 0, -sinf(player->yaw) };
    
    Vector3 moveDir = { 0 };
    if (input->upPressed) { moveDir = Vector3Add(moveDir, cameraForward); player->isMoving = true; }
    if (input->downPressed) { moveDir = Vector3Subtract(moveDir, cameraForward); player->isMoving = true; }
    if (input->leftPressed) { moveDir = Vector3Subtract(moveDir, cameraRight); player->isMoving = true; }
    if (input->rightPressed) { moveDir = Vector3Add(moveDir, cameraRight); player->isMoving = true; }
    
    if (player->isMoving) {
        moveDir = Vector3Normalize(moveDir);
        player->velocity = Vector3Scale(moveDir, PLAYER_SPEED);
        player->position = Vector3Add(player->position, Vector3Scale(player->velocity, dt));
        player->animTime += dt * 8.0f;
    }
    
    player->yaw += input->mouseDelta.x * 0.003f;
    player->pitch -= input->mouseDelta.y * 0.003f;
    player->pitch = Clamp(player->pitch, -PI / 2.0f + 0.1f, PI / 2.0f - 0.1f);
}

void PlayerRender(Player *player, Shader shader) {
    (void)shader;
    if (!player->bodyModel.meshCount) return;

    float walk = player->isMoving ? sinf(player->animTime) : 0.0f;
    float legSwing = walk * 0.5f;
    float armSwing = walk * 0.3f;

    Vector3 bodyPos = player->position;
    bodyPos.y += 1.0f;
    DrawModelEx(player->bodyModel, bodyPos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 headPos = Vector3Add(bodyPos, (Vector3){ 0, 0.7f, 0 });
    DrawModelEx(player->headModel, headPos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 leftShoulder = Vector3Add(bodyPos, (Vector3){ -0.5f, 0.4f, 0 });
    Vector3 rightShoulder = Vector3Add(bodyPos, (Vector3){ 0.5f, 0.4f, 0 });
    Vector3 leftHip = Vector3Add(bodyPos, (Vector3){ -0.2f, -0.6f, 0 });
    Vector3 rightHip = Vector3Add(bodyPos, (Vector3){ 0.2f, -0.6f, 0 });

    DrawModelEx(player->leftArmModel, leftShoulder, (Vector3){ 0, 1, 0 }, armSwing * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
    DrawModelEx(player->rightArmModel, rightShoulder, (Vector3){ 0, 1, 0 }, -armSwing * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
    DrawModelEx(player->leftLegModel, leftHip, (Vector3){ 0, 1, 0 }, -legSwing * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
    DrawModelEx(player->rightLegModel, rightHip, (Vector3){ 0, 1, 0 }, legSwing * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
}

void PlayerShutdown(Player *player) {
    UnloadModel(player->bodyModel);
    UnloadModel(player->headModel);
    UnloadModel(player->leftArmModel);
    UnloadModel(player->rightArmModel);
    UnloadModel(player->leftLegModel);
    UnloadModel(player->rightLegModel);
    UnloadTexture(player->uniformTex);
    UnloadTexture(player->skinTex);
}

void PlayerTakeDamage(Player *player, float damage) {
    player->health -= damage;
    if (player->health < 0) player->health = 0;
}

Vector3 PlayerGetForward(Player *player) {
    return (Vector3){
        sinf(player->yaw) * cosf(player->pitch),
        sinf(player->pitch),
        cosf(player->yaw) * cosf(player->pitch)
    };
}

Vector3 PlayerGetRight(Player *player) {
    return (Vector3){
        -cosf(player->yaw),
        0,
        sinf(player->yaw)
    };
}
