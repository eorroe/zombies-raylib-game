#include "player.h"
#include "raymath.h"
#include "texture.h"
#include "input.h"
#include "player_mesh.h"
#include <stdlib.h>
#include <math.h>

static Model CreateLimbPivoted(float radius, float length, int slices) {
    Mesh m = PlayerMesh_CreateLimb(radius, length, slices);
    for (int i = 0; i < m.vertexCount; i++) {
        m.vertices[i * 3 + 1] -= length * 0.5f;
    }
    PlayerMesh_Upload(&m);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateLimb(float radius, float length, int slices) {
    Mesh m = PlayerMesh_CreateLimb(radius, length, slices);
    PlayerMesh_Upload(&m);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateHand(float radius) {
    Mesh m = PlayerMesh_CreateHand(radius);
    PlayerMesh_Upload(&m);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateFoot(float radius) {
    Mesh m = PlayerMesh_CreateFoot(radius);
    PlayerMesh_Upload(&m);
    Model model = LoadModelFromMesh(m);
    return model;
}

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

void PlayerInit(Player *player, Vector3 startPos, Shader pbr) {
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

    Mesh bodyMesh = PlayerMesh_CreateTorso(0.5f, 1.4f, 0.3f);
    player->bodyModel = LoadModelFromMesh(bodyMesh);
    player->bodyModel.materials[0].shader = pbr;

    Mesh headMesh = PlayerMesh_CreateHead(0.28f);
    player->headModel = LoadModelFromMesh(headMesh);
    player->headModel.materials[0].shader = pbr;

    Mesh helmetMesh = PlayerMesh_CreateHelmet(0.32f);
    player->helmetModel = LoadModelFromMesh(helmetMesh);
    player->helmetModel.materials[0].shader = pbr;

    player->leftArmModel = CreateLimb(0.1f, 0.8f, 16);
    player->leftArmModel.materials[0].shader = pbr;
    player->rightArmModel = CreateLimb(0.1f, 0.8f, 16);
    player->rightArmModel.materials[0].shader = pbr;
    player->leftLegModel = CreateLimbPivoted(0.12f, 1.0f, 16);
    player->leftLegModel.materials[0].shader = pbr;
    player->rightLegModel = CreateLimbPivoted(0.12f, 1.0f, 16);
    player->rightLegModel.materials[0].shader = pbr;

    player->leftHandModel = CreateHand(0.06f);
    player->leftHandModel.materials[0].shader = pbr;
    player->rightHandModel = CreateHand(0.06f);
    player->rightHandModel.materials[0].shader = pbr;
    player->leftFootModel = CreateFoot(0.08f);
    player->leftFootModel.materials[0].shader = pbr;
    player->rightFootModel = CreateFoot(0.08f);
    player->rightFootModel.materials[0].shader = pbr;

    Image uniformImg = GenImageColor(256, 256, (Color){ 60, 100, 160, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 50 + rand() % 70;
        ImageDrawPixel(&uniformImg, x, y, (Color){ shade, shade + 20, shade + 60, 255 });
    }
    Texture2D uniformTex = LoadTextureFromImage(uniformImg);
    UnloadImage(uniformImg);

    Image skinImg = GenImageColor(256, 256, (Color){ 80, 120, 170, 255 });
    for (int i = 0; i < 1500; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 60 + rand() % 90;
        ImageDrawPixel(&skinImg, x, y, (Color){ shade, shade + 30, shade + 70, 255 });
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
    Vector3 cameraRight = (Vector3){ -cosf(player->yaw), 0, sinf(player->yaw) };
    
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

static void DrawStickLimb(Vector3 start, Vector3 end, float radius, Color color) {
    Vector3 dir = Vector3Subtract(end, start);
    float len = Vector3Length(dir);
    if (len < 0.001f) return;
    
    Vector3 mid = Vector3Add(start, Vector3Scale(dir, 0.5f));
    
    Vector3 up = (Vector3){ 0, 1, 0 };
    Vector3 axis = Vector3Normalize(dir);
    float dot = Vector3DotProduct(up, axis);
    if (dot > 1.0f) dot = 1.0f;
    if (dot < -1.0f) dot = -1.0f;
    
    float angle = acosf(dot) * RAD2DEG;
    Vector3 rotationAxis = Vector3CrossProduct(up, axis);
    if (Vector3Length(rotationAxis) < 0.001f) {
        rotationAxis = (Vector3){ 1, 0, 0 };
    } else {
        rotationAxis = Vector3Normalize(rotationAxis);
    }
    
    DrawCylinder3D(mid, radius, radius, len, 6, color);
    DrawSphere(start, radius * 1.2f, color);
    DrawSphere(end, radius * 1.2f, color);
}

void PlayerRender(Player *player, Shader shader) {
    (void)shader;
    
    float walk = player->isMoving ? sinf(player->animTime) : 0.0f;
    float armSwing = walk * 0.3f;

    float yawDeg = player->yaw * RAD2DEG;
    float cosYaw = cosf(player->yaw);
    float sinYaw = sinf(player->yaw);

    Vector3 bodyPos = player->position;
    bodyPos.y += 1.0f;
    
    Vector3 headPos = Vector3Add(bodyPos, (Vector3){ 0, 0.7f, 0 });
    Vector3 helmetPos = Vector3Add(headPos, (Vector3){ 0, 0.05f, 0 });
    
    Vector3 leftShoulder = Vector3Add(bodyPos, (Vector3){ -0.5f * cosYaw, 0.4f, 0.5f * sinYaw });
    Vector3 rightShoulder = Vector3Add(bodyPos, (Vector3){ 0.5f * cosYaw, 0.4f, -0.5f * sinYaw });
    Vector3 leftHip = Vector3Add(bodyPos, (Vector3){ -0.2f * cosYaw, -0.6f, 0.2f * sinYaw });
    Vector3 rightHip = Vector3Add(bodyPos, (Vector3){ 0.2f * cosYaw, -0.6f, -0.2f * sinYaw });

    Vector3 forward = PlayerGetForward(player);
    Vector3 right = PlayerGetRight(player);
    float fwd = Vector3DotProduct(player->moveDir, forward);
    float rightDot = Vector3DotProduct(player->moveDir, right);

    float legSwing = walk * 0.5f;
    Vector3 leftAxis = (Vector3){ 0, 0, 1 };
    Vector3 rightAxis = (Vector3){ 0, 0, 1 };

    if (player->isMoving) {
        float moveMag = sqrtf(fwd * fwd + rightDot * rightDot);
        if (moveMag > 0.001f) {
            fwd /= moveMag;
            rightDot /= moveMag;
        }

        if (fabsf(fwd) > fabsf(rightDot)) {
            if (fwd < 0.0f) legSwing = -legSwing;
            leftAxis = rightAxis = PlayerGetRight(player);
        } else {
            leftAxis = rightAxis = PlayerGetForward(player);
        }
    }

    float leftLegAngle = -legSwing * RAD2DEG;
    float rightLegAngle = legSwing * RAD2DEG;
    
    // Calculate limb endpoints
    Vector3 leftElbow = Vector3Add(leftShoulder, Vector3Scale(right, -0.35f));
    Vector3 rightElbow = Vector3Add(rightShoulder, Vector3Scale(right, -0.35f));
    Vector3 leftHandPos = Vector3Add(leftShoulder, Vector3Scale(right, -0.7f));
    Vector3 rightHandPos = Vector3Add(rightShoulder, Vector3Scale(right, -0.7f));
    Vector3 leftKnee = Vector3Add(leftHip, Vector3Scale(leftAxis, -PLAYER_LEG_LENGTH * 0.5f));
    Vector3 rightKnee = Vector3Add(rightHip, Vector3Scale(rightAxis, -PLAYER_LEG_LENGTH * 0.5f));
    Vector3 leftFootPos = Vector3Add(leftHip, Vector3Scale(leftAxis, -PLAYER_LEG_LENGTH));
    Vector3 rightFootPos = Vector3Add(rightHip, Vector3Scale(rightAxis, -PLAYER_LEG_LENGTH));
    
    // Draw head
    DrawSphere(helmetPos, 0.25f, (Color){ 20, 30, 60, 255 });
    DrawSphere(headPos, 0.2f, (Color){ 80, 120, 170, 255 });
    
    // Draw torso
    DrawStickLimb(leftShoulder, leftHip, 0.04f, (Color){ 60, 100, 160, 255 });
    DrawStickLimb(rightShoulder, rightHip, 0.04f, (Color){ 60, 100, 160, 255 });
    DrawStickLimb(leftShoulder, rightShoulder, 0.03f, (Color){ 60, 100, 160, 255 });
    
    // Draw arms
    DrawStickLimb(leftShoulder, leftElbow, 0.025f, (Color){ 60, 100, 160, 255 });
    DrawStickLimb(leftElbow, leftHandPos, 0.02f, (Color){ 80, 120, 170, 255 });
    DrawStickLimb(rightShoulder, rightElbow, 0.025f, (Color){ 60, 100, 160, 255 });
    DrawStickLimb(rightElbow, rightHandPos, 0.02f, (Color){ 80, 120, 170, 255 });
    
    // Draw legs
    DrawStickLimb(leftHip, leftKnee, 0.03f, (Color){ 30, 50, 100, 255 });
    DrawStickLimb(leftKnee, leftFootPos, 0.025f, (Color){ 30, 50, 100, 255 });
    DrawStickLimb(rightHip, rightKnee, 0.03f, (Color){ 30, 50, 100, 255 });
    DrawStickLimb(rightKnee, rightFootPos, 0.025f, (Color){ 30, 50, 100, 255 });
    
    // Draw joints
    DrawSphere(leftShoulder, 0.04f, (Color){ 60, 100, 160, 255 });
    DrawSphere(rightShoulder, 0.04f, (Color){ 60, 100, 160, 255 });
    DrawSphere(leftHip, 0.04f, (Color){ 30, 50, 100, 255 });
    DrawSphere(rightHip, 0.04f, (Color){ 30, 50, 100, 255 });
    DrawSphere(leftElbow, 0.03f, (Color){ 60, 100, 160, 255 });
    DrawSphere(rightElbow, 0.03f, (Color){ 60, 100, 160, 255 });
    DrawSphere(leftKnee, 0.03f, (Color){ 30, 50, 100, 255 });
    DrawSphere(rightKnee, 0.03f, (Color){ 30, 50, 100, 255 });
    DrawSphere(leftHandPos, 0.03f, (Color){ 80, 120, 170, 255 });
    DrawSphere(rightHandPos, 0.03f, (Color){ 80, 120, 170, 255 });
    DrawSphere(leftFootPos, 0.035f, (Color){ 30, 50, 100, 255 });
    DrawSphere(rightFootPos, 0.035f, (Color){ 30, 50, 100, 255 });
}

void PlayerShutdown(Player *player) {
    UnloadModel(player->bodyModel);
    UnloadModel(player->headModel);
    UnloadModel(player->helmetModel);
    UnloadModel(player->leftArmModel);
    UnloadModel(player->rightArmModel);
    UnloadModel(player->leftLegModel);
    UnloadModel(player->rightLegModel);
    UnloadModel(player->leftHandModel);
    UnloadModel(player->rightHandModel);
    UnloadModel(player->leftFootModel);
    UnloadModel(player->rightFootModel);
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
