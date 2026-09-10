#include "player.h"
#include "zombie_mesh.h"
#include "raymath.h"
#include "texture.h"
#include "input.h"
#include <stdlib.h>
#include <math.h>

static Model LoadHighPolyModel(Mesh mesh) {
    ZombieMesh_Upload(&mesh);
    return LoadModelFromMesh(mesh);
}

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

static Vector3 RotateOffsetY(Vector3 offset, float cosYaw, float sinYaw) {
    return (Vector3){
        offset.x * cosYaw + offset.z * sinYaw,
        offset.y,
        -offset.x * sinYaw + offset.z * cosYaw
    };
}

static void DrawLimb(Model model, Vector3 origin, Vector3 offsetDir, Vector3 rotationAxis, float angle, float length) {
    Vector3 mid = Vector3Add(origin, Vector3Scale(offsetDir, length * 0.5f));
    DrawModelEx(model, mid, rotationAxis, angle * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
}

static void DrawBone(Model model, Vector3 start, Vector3 end, float meshLength, Color color) {
    Vector3 mid = Vector3Scale(Vector3Add(start, end), 0.5f);
    Vector3 dir = Vector3Normalize(Vector3Subtract(end, start));
    float segmentLength = Vector3Length(Vector3Subtract(end, start));

    Vector3 up = (Vector3){0, 1, 0};
    Vector3 axis = Vector3CrossProduct(up, dir);
    float angle = 0.0f;
    if (Vector3Length(axis) > 0.001f) {
        axis = Vector3Normalize(axis);
        angle = acosf(Clamp(Vector3DotProduct(up, dir), -1.0f, 1.0f)) * RAD2DEG;
    }

    float scaleY = segmentLength / meshLength;
    DrawModelEx(model, mid, axis, angle, (Vector3){ 1, scaleY, 1 }, color);
}

void PlayerInit(Player *player, Vector3 startPos, Shader pbr, ProceduralTextures *textures) {
    player->position = startPos;
    player->velocity = (Vector3){ 0 };
    player->health = PLAYER_HEALTH;
    player->maxHealth = PLAYER_HEALTH;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->isMoving = false;
    player->footstepTimer = 0.0f;
    player->animTime = 0.0f;
    player->moveDir = (Vector3){ 0 };

    Mesh torsoMesh = ZombieMesh_CreateTorso(0.55f, 0.85f, 0.5f);
    player->bodyModel = LoadHighPolyModel(torsoMesh);
    player->bodyModel.materials[0].shader = pbr;

    Mesh spineMesh = ZombieMesh_CreateSpine(0.7f);
    player->spineModel = LoadHighPolyModel(spineMesh);
    player->spineModel.materials[0].shader = pbr;

    Mesh ribMesh = ZombieMesh_CreateRibcage(0.5f, 0.6f);
    player->ribcageModel = LoadHighPolyModel(ribMesh);
    player->ribcageModel.materials[0].shader = pbr;

    Mesh pelvisMesh = ZombieMesh_CreatePelvis(0.45f, 0.35f);
    player->pelvisModel = LoadHighPolyModel(pelvisMesh);
    player->pelvisModel.materials[0].shader = pbr;

    Mesh headMesh = ZombieMesh_CreateHead(0.22f);
    player->headModel = LoadHighPolyModel(headMesh);
    player->headModel.materials[0].shader = pbr;

    Mesh jawMesh = ZombieMesh_CreateJaw(0.22f);
    player->jawModel = LoadHighPolyModel(jawMesh);
    player->jawModel.materials[0].shader = pbr;

    Mesh leftUpperArmMesh = ZombieMesh_CreateLimb(0.08f, 0.55f);
    player->leftUpperArm = LoadHighPolyModel(leftUpperArmMesh);
    player->leftUpperArm.materials[0].shader = pbr;

    Mesh leftLowerArmMesh = ZombieMesh_CreateLimb(0.06f, 0.5f);
    player->leftLowerArm = LoadHighPolyModel(leftLowerArmMesh);
    player->leftLowerArm.materials[0].shader = pbr;

    Mesh rightUpperArmMesh = ZombieMesh_CreateLimb(0.08f, 0.55f);
    player->rightUpperArm = LoadHighPolyModel(rightUpperArmMesh);
    player->rightUpperArm.materials[0].shader = pbr;

    Mesh rightLowerArmMesh = ZombieMesh_CreateLimb(0.06f, 0.5f);
    player->rightLowerArm = LoadHighPolyModel(rightLowerArmMesh);
    player->rightLowerArm.materials[0].shader = pbr;

    Mesh leftUpperLegMesh = ZombieMesh_CreateLimb(0.1f, 0.45f);
    player->leftUpperLeg = LoadHighPolyModel(leftUpperLegMesh);
    player->leftUpperLeg.materials[0].shader = pbr;

    Mesh leftLowerLegMesh = ZombieMesh_CreateLimb(0.08f, 0.45f);
    player->leftLowerLeg = LoadHighPolyModel(leftLowerLegMesh);
    player->leftLowerLeg.materials[0].shader = pbr;

    Mesh rightUpperLegMesh = ZombieMesh_CreateLimb(0.1f, 0.45f);
    player->rightUpperLeg = LoadHighPolyModel(rightUpperLegMesh);
    player->rightUpperLeg.materials[0].shader = pbr;

    Mesh rightLowerLegMesh = ZombieMesh_CreateLimb(0.08f, 0.45f);
    player->rightLowerLeg = LoadHighPolyModel(rightLowerLegMesh);
    player->rightLowerLeg.materials[0].shader = pbr;

    Mesh leftHandMesh = ZombieMesh_CreateHand(0.18f);
    player->leftHandModel = LoadHighPolyModel(leftHandMesh);
    player->leftHandModel.materials[0].shader = pbr;

    Mesh rightHandMesh = ZombieMesh_CreateHand(0.18f);
    player->rightHandModel = LoadHighPolyModel(rightHandMesh);
    player->rightHandModel.materials[0].shader = pbr;

    Mesh leftFootMesh = ZombieMesh_CreateFoot(0.22f);
    player->leftFootModel = LoadHighPolyModel(leftFootMesh);
    player->leftFootModel.materials[0].shader = pbr;

    Mesh rightFootMesh = ZombieMesh_CreateFoot(0.22f);
    player->rightFootModel = LoadHighPolyModel(rightFootMesh);
    player->rightFootModel.materials[0].shader = pbr;

    Image skinImg = GenImageColor(256, 256, (Color){ 220, 200, 170, 255 });
    for (int i = 0; i < 1500; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 180 + rand() % 60;
        ImageDrawPixel(&skinImg, x, y, (Color){ shade + 20, shade - 5, shade - 20, 255 });
    }
    for (int i = 0; i < 60; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int r = 3 + rand() % 8;
        ImageDrawCircle(&skinImg, x, y, r, (Color){ 160, 100, 80, 180 });
    }
    Texture2D skinTex = LoadTextureFromImage(skinImg);
    UnloadImage(skinImg);
    player->skinTex = skinTex;

    Texture2D camoTex = textures->camo;
    player->camoTex = camoTex;

    SetModelTexture(&player->bodyModel, camoTex);
    SetModelTexture(&player->spineModel, textures->zombieBone);
    SetModelTexture(&player->ribcageModel, textures->zombieBone);
    SetModelTexture(&player->pelvisModel, textures->zombieBone);
    SetModelTexture(&player->headModel, skinTex);
    SetModelTexture(&player->jawModel, skinTex);
    SetModelTexture(&player->leftUpperArm, camoTex);
    SetModelTexture(&player->leftLowerArm, camoTex);
    SetModelTexture(&player->rightUpperArm, camoTex);
    SetModelTexture(&player->rightLowerArm, camoTex);
    SetModelTexture(&player->leftUpperLeg, camoTex);
    SetModelTexture(&player->leftLowerLeg, camoTex);
    SetModelTexture(&player->rightUpperLeg, camoTex);
    SetModelTexture(&player->rightLowerLeg, camoTex);
    SetModelTexture(&player->leftHandModel, skinTex);
    SetModelTexture(&player->rightHandModel, skinTex);
    SetModelTexture(&player->leftFootModel, camoTex);
    SetModelTexture(&player->rightFootModel, camoTex);
}

void PlayerUpdate(Player *player, InputState *input, float dt) {
    player->velocity = (Vector3){ 0 };
    player->isMoving = false;
    
    Vector3 cameraForward = (Vector3){ sinf(player->yaw), 0, cosf(player->yaw) };
    Vector3 cameraRight = (Vector3){ -cosf(player->yaw), 0, sinf(player->yaw) };
    
    Vector3 moveDir = { 0 };
    if (input->upPressed) { moveDir = Vector3Add(moveDir, cameraForward); player->isMoving = true; }
    if (input->downPressed) { moveDir = Vector3Subtract(moveDir, cameraForward); player->isMoving = true; }
    if (input->leftPressed) { moveDir = Vector3Subtract(moveDir, cameraRight); player->isMoving = true; }
    if (input->rightPressed) { moveDir = Vector3Add(moveDir, cameraRight); player->isMoving = true; }
    
    if (player->isMoving) {
        moveDir = Vector3Normalize(moveDir);
        float speed = PLAYER_SPEED;
        if (input->shiftPressed) speed *= 2.0f;
        player->velocity = Vector3Scale(moveDir, speed);
        player->position = Vector3Add(player->position, Vector3Scale(player->velocity, dt));
        player->animTime += dt * 8.0f;
        player->moveDir = moveDir;
    } else {
        player->moveDir = (Vector3){ 0 };
    }
    
    Vector2 mouseDelta = InputGetMouseDelta(input);
    if (fabsf(mouseDelta.x) < 100.0f && fabsf(mouseDelta.y) < 100.0f) {
        player->yaw -= mouseDelta.x * 0.003f;
        player->pitch -= mouseDelta.y * 0.003f;
    }
    player->pitch = Clamp(player->pitch, -PI / 2.0f + 0.1f, PI / 2.0f - 0.1f);
}

void PlayerRender(Player *player, Shader shader) {
    (void)shader;
    if (!player->bodyModel.meshCount) return;

    float walk = player->isMoving ? sinf(player->animTime) : 0.0f;
    float armSwing = walk * 0.5f;
    float legSwing = walk * 0.6f;

    float yawDeg = player->yaw * RAD2DEG;
    float cosYaw = cosf(player->yaw);
    float sinYaw = sinf(player->yaw);
    Vector3 zombieRight = (Vector3){ cosYaw, 0.0f, -sinYaw };

    float legUpperLen = 0.45f;
    float legLowerLen = 0.45f;
    float torsoHeight = 0.85f;
    float hipY = player->position.y + legUpperLen + legLowerLen;
    float torsoCenterY = hipY + torsoHeight * 0.5f;
    float headCenterY = hipY + torsoHeight + 0.22f * 0.9f;

    Vector3 torsoOffset = (Vector3){ 0.0f, torsoCenterY, 0.0f };
    Vector3 torsoPos = Vector3Add(player->position, RotateOffsetY(torsoOffset, cosYaw, sinYaw));
    float bodyY = torsoPos.y;

    DrawModelEx(player->bodyModel, (Vector3){ torsoPos.x, bodyY, torsoPos.z }, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 spineOffset = (Vector3){ 0.0f, bodyY - torsoPos.y - torsoHeight * 0.15f, 0.0f };
    Vector3 spinePos = Vector3Add(torsoPos, RotateOffsetY(spineOffset, cosYaw, sinYaw));
    DrawModelEx(player->spineModel, spinePos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 ribOffset = (Vector3){ 0.0f, bodyY - torsoPos.y + torsoHeight * 0.05f, 0.0f };
    Vector3 ribPos = Vector3Add(torsoPos, RotateOffsetY(ribOffset, cosYaw, sinYaw));
    DrawModelEx(player->ribcageModel, ribPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 pelvisOffset = (Vector3){ 0.0f, bodyY - torsoPos.y - torsoHeight * 0.35f, 0.0f };
    Vector3 pelvisPos = Vector3Add(torsoPos, RotateOffsetY(pelvisOffset, cosYaw, sinYaw));
    DrawModelEx(player->pelvisModel, pelvisPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 headOffset = (Vector3){ 0.0f, headCenterY - torsoPos.y, 0.0f };
    Vector3 headPos = Vector3Add(torsoPos, RotateOffsetY(headOffset, cosYaw, sinYaw));
    DrawModelEx(player->headModel, (Vector3){ headPos.x, headPos.y, headPos.z }, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 jawOffset = (Vector3){ 0.0f, -0.22f * 0.3f, 0.22f * 0.4f };
    Vector3 jawPos = Vector3Add(headPos, RotateOffsetY(jawOffset, cosYaw, sinYaw));
    DrawModelEx(player->jawModel, (Vector3){ jawPos.x, jawPos.y, jawPos.z }, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 shoulderLOffset = (Vector3){ -0.55f * 0.6f, torsoCenterY + torsoHeight * 0.35f - torsoPos.y, 0.0f };
    Vector3 shoulderROffset = (Vector3){ 0.55f * 0.6f, torsoCenterY + torsoHeight * 0.35f - torsoPos.y, 0.0f };
    Vector3 hipLOffset = (Vector3){ -0.55f * 0.35f, hipY - torsoPos.y, 0.0f };
    Vector3 hipROffset = (Vector3){ 0.55f * 0.35f, hipY - torsoPos.y, 0.0f };

    Vector3 shoulderL = Vector3Add(torsoPos, RotateOffsetY(shoulderLOffset, cosYaw, sinYaw));
    Vector3 shoulderR = Vector3Add(torsoPos, RotateOffsetY(shoulderROffset, cosYaw, sinYaw));
    Vector3 hipL = Vector3Add(torsoPos, RotateOffsetY(hipLOffset, cosYaw, sinYaw));
    Vector3 hipR = Vector3Add(torsoPos, RotateOffsetY(hipROffset, cosYaw, sinYaw));

    float armAngleX = -0.6f;
    float armAngleY = -0.8f;
    Vector3 armOffsetDir = (Vector3){ armAngleX, armAngleY, 0.0f };
    Vector3 armOffsetDirRotated = RotateOffsetY(armOffsetDir, cosYaw, sinYaw);
    Vector3 armOffsetDirRight = RotateOffsetY((Vector3){ -armAngleX, armAngleY, 0.0f }, cosYaw, sinYaw);
    Vector3 legOffsetDirUpper = (Vector3){ -0.2f, -1.0f, 0.0f };
    Vector3 legOffsetDirLower = (Vector3){ -0.15f, -1.0f, 0.0f };
    Vector3 legOffsetDirRotatedUpper = RotateOffsetY(legOffsetDirUpper, cosYaw, sinYaw);
    Vector3 legOffsetDirRotatedLower = RotateOffsetY(legOffsetDirLower, cosYaw, sinYaw);

    Vector3 elbowL = Vector3Add(shoulderL, Vector3Scale(armOffsetDirRotated, 0.55f));
    Vector3 elbowR = Vector3Add(shoulderR, Vector3Scale(armOffsetDirRight, 0.55f));

    DrawBone(player->leftUpperArm, shoulderL, elbowL, 0.55f, WHITE);
    DrawBone(player->leftLowerArm, elbowL, Vector3Add(elbowL, Vector3Scale(armOffsetDirRotated, 0.5f)), 0.5f, WHITE);

    DrawBone(player->rightUpperArm, shoulderR, elbowR, 0.55f, WHITE);
    DrawBone(player->rightLowerArm, elbowR, Vector3Add(elbowR, Vector3Scale(armOffsetDirRight, 0.5f)), 0.5f, WHITE);

    DrawLimb(player->leftUpperLeg, hipL, legOffsetDirRotatedUpper, zombieRight, legSwing, 0.45f);
    DrawLimb(player->leftLowerLeg, Vector3Add(hipL, RotateOffsetY((Vector3){ -0.2f * 0.45f, -1.0f * 0.45f, 0.0f }, cosYaw, sinYaw)), legOffsetDirRotatedLower, zombieRight, legSwing * 1.2f, 0.45f);

    DrawLimb(player->rightUpperLeg, hipR, RotateOffsetY((Vector3){ 0.2f, -1.0f, 0.0f }, cosYaw, sinYaw), zombieRight, -legSwing, 0.45f);
    DrawLimb(player->rightLowerLeg, Vector3Add(hipR, RotateOffsetY((Vector3){ 0.2f * 0.45f, -1.0f * 0.45f, 0.0f }, cosYaw, sinYaw)), RotateOffsetY((Vector3){ 0.15f, -1.0f, 0.0f }, cosYaw, sinYaw), zombieRight, -legSwing * 1.2f, 0.45f);

    Vector3 wristL = Vector3Add(elbowL, Vector3Scale(armOffsetDirRotated, 0.5f));
    Vector3 wristR = Vector3Add(elbowR, Vector3Scale(armOffsetDirRight, 0.5f));
    Vector3 leftHandPos = wristL;
    Vector3 rightHandPos = wristR;
    DrawModelEx(player->leftHandModel, leftHandPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);
    DrawModelEx(player->rightHandModel, rightHandPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);

    Vector3 leftFootPos = Vector3Add(hipL, RotateOffsetY((Vector3){ -0.2f * 0.45f, -1.0f * 0.45f - 0.45f * 0.5f, 0.0f }, cosYaw, sinYaw));
    Vector3 rightFootPos = Vector3Add(hipR, RotateOffsetY((Vector3){ 0.2f * 0.45f, -1.0f * 0.45f - 0.45f * 0.5f, 0.0f }, cosYaw, sinYaw));
    DrawModelEx(player->leftFootModel, leftFootPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);
    DrawModelEx(player->rightFootModel, rightFootPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, WHITE);
}

void PlayerShutdown(Player *player) {
    UnloadModel(player->bodyModel);
    UnloadModel(player->headModel);
    UnloadModel(player->jawModel);
    UnloadModel(player->spineModel);
    UnloadModel(player->ribcageModel);
    UnloadModel(player->pelvisModel);
    UnloadModel(player->leftUpperArm);
    UnloadModel(player->leftLowerArm);
    UnloadModel(player->rightUpperArm);
    UnloadModel(player->rightLowerArm);
    UnloadModel(player->leftUpperLeg);
    UnloadModel(player->leftLowerLeg);
    UnloadModel(player->rightUpperLeg);
    UnloadModel(player->rightLowerLeg);
    UnloadModel(player->leftHandModel);
    UnloadModel(player->rightHandModel);
    UnloadModel(player->leftFootModel);
    UnloadModel(player->rightFootModel);
    UnloadTexture(player->uniformTex);
    UnloadTexture(player->skinTex);
    UnloadTexture(player->camoTex);
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
        -cosf(player->yaw),
        0,
        sinf(player->yaw)
    };
}
