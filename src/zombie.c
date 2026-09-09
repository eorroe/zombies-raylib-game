#include "zombie.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

static Model CreateLimbMesh(float radius, float length, int slices) {
    Mesh m = GenMeshCylinder(radius, length, slices);
    Model model = LoadModelFromMesh(m);
    return model;
}

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex, Texture2D skin, Texture2D shirt, Texture2D pants) {
    zombie->position = position;
    zombie->velocity = (Vector3){ 0 };
    zombie->health = 50.0f + rand() % 50;
    zombie->maxHealth = zombie->health;
    zombie->type = type;
    zombie->textureIndex = textureIndex;
    zombie->attackCooldown = 0.0f;
    zombie->active = true;
    zombie->animTime = (float)rand() / RAND_MAX * 6.28f;
    zombie->walkCycle = (float)rand() / RAND_MAX * 6.28f;
    zombie->skinTex = skin;
    zombie->shirtTex = shirt;
    zombie->pantsTex = pants;

    Mesh torsoMesh = GenMeshCylinder(TORSO_WIDTH, TORSO_HEIGHT, 8);
    zombie->bodyModel = LoadModelFromMesh(torsoMesh);
    SetModelTexture(&zombie->bodyModel, shirt);

    Mesh headMesh = GenMeshSphere(HEAD_RADIUS, 12, 12);
    zombie->headModel = LoadModelFromMesh(headMesh);
    SetModelTexture(&zombie->headModel, skin);

    zombie->leftUpperArm = CreateLimbMesh(LIMB_RADIUS, ARM_UPPER_LEN, 8);
    SetModelTexture(&zombie->leftUpperArm, shirt);
    zombie->leftLowerArm = CreateLimbMesh(LIMB_RADIUS * 0.8f, ARM_LOWER_LEN, 8);
    SetModelTexture(&zombie->leftLowerArm, skin);
    zombie->rightUpperArm = CreateLimbMesh(LIMB_RADIUS, ARM_UPPER_LEN, 8);
    SetModelTexture(&zombie->rightUpperArm, shirt);
    zombie->rightLowerArm = CreateLimbMesh(LIMB_RADIUS * 0.8f, ARM_LOWER_LEN, 8);
    SetModelTexture(&zombie->rightLowerArm, skin);

    zombie->leftUpperLeg = CreateLimbMesh(LIMB_RADIUS * 1.1f, LEG_UPPER_LEN, 8);
    SetModelTexture(&zombie->leftUpperLeg, pants);
    zombie->leftLowerLeg = CreateLimbMesh(LIMB_RADIUS * 0.9f, LEG_LOWER_LEN, 8);
    SetModelTexture(&zombie->leftLowerLeg, pants);
    zombie->rightUpperLeg = CreateLimbMesh(LIMB_RADIUS * 1.1f, LEG_UPPER_LEN, 8);
    SetModelTexture(&zombie->rightUpperLeg, pants);
    zombie->rightLowerLeg = CreateLimbMesh(LIMB_RADIUS * 0.9f, LEG_LOWER_LEN, 8);
    SetModelTexture(&zombie->rightLowerLeg, pants);
}

void ZombieUpdate(Zombie *zombie, Vector3 playerPos, float dt) {
    if (!zombie->active) return;
    zombie->animTime += dt * 3.0f;
    zombie->walkCycle += dt * 6.0f;
    Vector3 dir = Vector3Subtract(playerPos, zombie->position);
    float dist = Vector3Length(dir);
    if (dist > 0.1f) {
        dir = Vector3Normalize(dir);
        zombie->velocity = Vector3Scale(dir, ZOMBIE_SPEED_BASE);
        zombie->position = Vector3Add(zombie->position, Vector3Scale(zombie->velocity, dt));
    }
    if (zombie->attackCooldown > 0) zombie->attackCooldown -= dt;
}

static void DrawLimb(Model model, Vector3 origin, Vector3 axis, float angle, float length) {
    Vector3 mid = Vector3Add(origin, Vector3Scale(axis, length * 0.5f));
    DrawModelEx(model, mid, (Vector3){ 0, 0, 1 }, angle * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
}

void ZombieRender(Zombie *zombie, Camera3D camera, Texture2D *headTextures, int headTextureCount, Shader shader) {
    if (!zombie->active) return;
    float bob = sinf(zombie->animTime) * 0.05f;
    float walk = sinf(zombie->walkCycle);
    float feetY = zombie->position.y + bob;

    Color skinColor = (Color){ 255, 100, 100, 255 };
    Color shirtColor = (Color){ 255, 30, 30, 255 };
    Color pantsColor = (Color){ 30, 30, 255, 255 };
    
    float hipY = feetY + LEG_UPPER_LEN + LEG_LOWER_LEN;
    float torsoCenterY = hipY + TORSO_HEIGHT * 0.5f;
    float headCenterY = hipY + TORSO_HEIGHT + HEAD_RADIUS * 0.9f;

    Vector3 torsoPos = (Vector3){ zombie->position.x, torsoCenterY, zombie->position.z };
    SetModelTexture(&zombie->bodyModel, zombie->shirtTex);
    DrawModelEx(zombie->bodyModel, torsoPos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 255, 50, 50, 255 });

    Vector3 headPos = (Vector3){ zombie->position.x, headCenterY, zombie->position.z };
    if (zombie->type == ZOMBIE_TYPE_IMAGE_HEAD && headTextureCount > 0 && zombie->textureIndex < headTextureCount) {
        if (headTextures[zombie->textureIndex].id != 0) {
            SetModelTexture(&zombie->headModel, headTextures[zombie->textureIndex]);
        } else {
            SetModelTexture(&zombie->headModel, zombie->skinTex);
        }
    } else {
        SetModelTexture(&zombie->headModel, zombie->skinTex);
    }
    DrawModelEx(zombie->headModel, headPos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 255, 80, 80, 255 });

    Vector3 shoulderL = (Vector3){ zombie->position.x - TORSO_WIDTH * 0.6f, torsoCenterY + TORSO_HEIGHT * 0.35f, zombie->position.z };
    Vector3 shoulderR = (Vector3){ zombie->position.x + TORSO_WIDTH * 0.6f, torsoCenterY + TORSO_HEIGHT * 0.35f, zombie->position.z };
    Vector3 hipL = (Vector3){ zombie->position.x - TORSO_WIDTH * 0.35f, hipY, zombie->position.z };
    Vector3 hipR = (Vector3){ zombie->position.x + TORSO_WIDTH * 0.35f, hipY, zombie->position.z };

    float armSwing = walk * 0.5f;
    float legSwing = walk * 0.6f;

    Vector3 playerPos = camera.position;
    float distToPlayer = Vector3Length(Vector3Subtract(playerPos, zombie->position));
    bool reaching = distToPlayer < REACH_DIST;
    float armAngleX = reaching ? -1.2f : -0.6f;
    float armAngleY = reaching ? 0.3f : -0.8f;

    DrawLimb(zombie->leftUpperArm, shoulderL, (Vector3){ armAngleX, armAngleY, 0 }, armSwing, ARM_UPPER_LEN);
    DrawLimb(zombie->leftLowerArm, Vector3Add(shoulderL, (Vector3){ armAngleX * ARM_UPPER_LEN, armAngleY * ARM_UPPER_LEN, 0 }), (Vector3){ armAngleX * 0.7f, armAngleY * 0.8f, 0 }, armSwing * 1.3f, ARM_LOWER_LEN);

    DrawLimb(zombie->rightUpperArm, shoulderR, (Vector3){ -armAngleX, armAngleY, 0 }, -armSwing, ARM_UPPER_LEN);
    DrawLimb(zombie->rightLowerArm, Vector3Add(shoulderR, (Vector3){ -armAngleX * ARM_UPPER_LEN, armAngleY * ARM_UPPER_LEN, 0 }), (Vector3){ -armAngleX * 0.7f, armAngleY * 0.8f, 0 }, -armSwing * 1.3f, ARM_LOWER_LEN);

    DrawLimb(zombie->leftUpperLeg, hipL, (Vector3){ -0.2f, -1.0f, 0 }, -legSwing, LEG_UPPER_LEN);
    DrawLimb(zombie->leftLowerLeg, Vector3Add(hipL, (Vector3){ -0.2f * LEG_UPPER_LEN, -1.0f * LEG_UPPER_LEN, 0 }), (Vector3){ -0.15f, -1.0f, 0 }, -legSwing * 1.2f, LEG_LOWER_LEN);

    DrawLimb(zombie->rightUpperLeg, hipR, (Vector3){ 0.2f, -1.0f, 0 }, legSwing, LEG_UPPER_LEN);
    DrawLimb(zombie->rightLowerLeg, Vector3Add(hipR, (Vector3){ 0.2f * LEG_UPPER_LEN, -1.0f * LEG_UPPER_LEN, 0 }), (Vector3){ 0.15f, -1.0f, 0 }, legSwing * 1.2f, LEG_LOWER_LEN);
}

void ZombieShutdown(Zombie *zombie) {
    UnloadModel(zombie->bodyModel);
    UnloadModel(zombie->headModel);
    UnloadModel(zombie->leftUpperArm);
    UnloadModel(zombie->leftLowerArm);
    UnloadModel(zombie->rightUpperArm);
    UnloadModel(zombie->rightLowerArm);
    UnloadModel(zombie->leftUpperLeg);
    UnloadModel(zombie->leftLowerLeg);
    UnloadModel(zombie->rightUpperLeg);
    UnloadModel(zombie->rightLowerLeg);
}

bool ZombieIsAlive(Zombie *zombie) {
    return zombie->active && zombie->health > 0;
}

void ZombieTakeDamage(Zombie *zombie, float damage) {
    zombie->health -= damage;
    if (zombie->health <= 0) {
        zombie->active = false;
    }
}
