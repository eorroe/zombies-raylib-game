#include "zombie.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

static Model CreateLimbMesh(float radius, float length, int slices) {
    Mesh m = GenMeshCylinder(radius, length, slices);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateBoneMesh(float radius, float length, int slices) {
    Mesh m = GenMeshCylinder(radius, length, slices);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateJointMesh(float radius, int rings, int slices) {
    Mesh m = GenMeshSphere(radius, rings, slices);
    Model model = LoadModelFromMesh(m);
    return model;
}

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

static Model CreateSkullMesh(void) {
    Mesh m = GenMeshSphere(0.22f, 10, 10);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateRibcageMesh(void) {
    Mesh m = GenMeshCylinder(0.23f, 0.55f, 10);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreatePelvisMesh(void) {
    Mesh m = GenMeshCylinder(0.19f, 0.12f, 10);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateHandMesh(void) {
    Mesh m = GenMeshCube(0.07f, 0.10f, 0.03f);
    Model model = LoadModelFromMesh(m);
    return model;
}

static Model CreateFootMesh(void) {
    Mesh m = GenMeshCube(0.09f, 0.05f, 0.18f);
    Model model = LoadModelFromMesh(m);
    return model;
}

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex, Texture2D skin, Texture2D shirt, Texture2D pants, Texture2D bone) {
    zombie->position = position;
    zombie->velocity = (Vector3){ 0 };
    zombie->health = 50.0f + rand() % 50;
    zombie->maxHealth = zombie->health;
    zombie->type = type;
    zombie->textureIndex = textureIndex;
    zombie->attackCooldown = 0.0f;
    zombie->active = true;
    zombie->dying = false;
    zombie->deathTimer = 0.0f;
    zombie->animTime = (float)rand() / RAND_MAX * 6.28f;
    zombie->walkCycle = (float)rand() / RAND_MAX * 6.28f;
    zombie->skinTex = skin;
    zombie->shirtTex = shirt;
    zombie->pantsTex = pants;
    zombie->boneTex = bone;
    zombie->speed = ZOMBIE_SPEED_BASE;
    zombie->damageFlashTimer = 0.0f;

    zombie->skullModel = CreateSkullMesh();
    SetModelTexture(&zombie->skullModel, bone);

    zombie->spineModel = CreateBoneMesh(0.08f, 0.7f, 8);
    SetModelTexture(&zombie->spineModel, bone);

    zombie->ribcageModel = CreateRibcageMesh();
    SetModelTexture(&zombie->ribcageModel, bone);

    zombie->pelvisModel = CreatePelvisMesh();
    SetModelTexture(&zombie->pelvisModel, bone);

    zombie->leftUpperArm = CreateLimbMesh(0.04f, 0.55f, 8);
    SetModelTexture(&zombie->leftUpperArm, bone);
    zombie->leftLowerArm = CreateLimbMesh(0.035f, 0.50f, 8);
    SetModelTexture(&zombie->leftLowerArm, bone);
    zombie->rightUpperArm = CreateLimbMesh(0.04f, 0.55f, 8);
    SetModelTexture(&zombie->rightUpperArm, bone);
    zombie->rightLowerArm = CreateLimbMesh(0.035f, 0.50f, 8);
    SetModelTexture(&zombie->rightLowerArm, bone);

    zombie->leftUpperLeg = CreateLimbMesh(0.055f, 0.65f, 8);
    SetModelTexture(&zombie->leftUpperLeg, bone);
    zombie->leftLowerLeg = CreateLimbMesh(0.045f, 0.60f, 8);
    SetModelTexture(&zombie->leftLowerLeg, bone);
    zombie->rightUpperLeg = CreateLimbMesh(0.055f, 0.65f, 8);
    SetModelTexture(&zombie->rightUpperLeg, bone);
    zombie->rightLowerLeg = CreateLimbMesh(0.045f, 0.60f, 8);
    SetModelTexture(&zombie->rightLowerLeg, bone);

    zombie->leftHand = CreateHandMesh();
    SetModelTexture(&zombie->leftHand, bone);
    zombie->rightHand = CreateHandMesh();
    SetModelTexture(&zombie->rightHand, bone);
    zombie->leftFoot = CreateFootMesh();
    SetModelTexture(&zombie->leftFoot, bone);
    zombie->rightFoot = CreateFootMesh();
    SetModelTexture(&zombie->rightFoot, bone);
}

void ZombieUpdate(Zombie *zombie, Vector3 playerPos, float dt, bool firstShotFired) {
    if (!zombie->active) return;
    zombie->animTime += dt * 3.0f;
    if (zombie->dying) {
        zombie->deathTimer -= dt;
        if (zombie->deathTimer <= 0.0f) {
            zombie->deathTimer = 0.0f;
        }
        if (zombie->attackCooldown > 0) zombie->attackCooldown -= dt;
        if (zombie->damageFlashTimer > 0.0f) zombie->damageFlashTimer -= dt;
        return;
    }
    if (zombie->health <= 0.0f) {
        if (zombie->attackCooldown > 0) zombie->attackCooldown -= dt;
        if (zombie->damageFlashTimer > 0.0f) zombie->damageFlashTimer -= dt;
        return;
    }
    if (firstShotFired) {
        zombie->walkCycle += dt * 6.0f;
        Vector3 dir = Vector3Subtract(playerPos, zombie->position);
        float dist = Vector3Length(dir);
        if (dist > 0.1f) {
            dir = Vector3Normalize(dir);
            zombie->velocity = Vector3Scale(dir, zombie->speed);
            zombie->position = Vector3Add(zombie->position, Vector3Scale(zombie->velocity, dt));
        }
    }
    if (zombie->attackCooldown > 0) zombie->attackCooldown -= dt;
    if (zombie->damageFlashTimer > 0.0f) zombie->damageFlashTimer -= dt;
}

static void DrawJoint(Vector3 pos, float radius, Color color) {
    DrawSphere(pos, radius, color);
}

static void DrawLimb(Model model, Vector3 origin, Vector3 axis, float angle, float length) {
    Vector3 mid = Vector3Add(origin, Vector3Scale(axis, length * 0.5f));
    DrawModelEx(model, mid, (Vector3){ 0, 0, 1 }, angle * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
}

void ZombieRender(Zombie *zombie, Camera3D camera, Texture2D *headTextures, int headTextureCount, Shader shader) {
    (void)shader;
    if (!zombie->active) return;
    float bob = sinf(zombie->animTime) * 0.05f;
    float walk = sinf(zombie->walkCycle);
    float feetY = zombie->position.y + bob;

    float flash = 0.0f;
    if (zombie->damageFlashTimer > 0.0f) {
        float cycle = fmodf(zombie->damageFlashTimer, 1.0f);
        if (cycle < 0.5f) flash = 1.0f;
    }
    
    Color boneColor = (Color){ 235, 225, 210, 255 };
    Color flashRed = (Color){ 120, 20, 20, 255 };
    
    Color bodyColor = (Color){
        (unsigned char)(boneColor.r + (flashRed.r - boneColor.r) * flash),
        (unsigned char)(boneColor.g + (flashRed.g - boneColor.g) * flash),
        (unsigned char)(boneColor.b + (flashRed.b - boneColor.b) * flash),
        255
    };
    Color headColor = bodyColor;
    
    float hipY = feetY + 0.65f + 0.60f;
    float pelvisY = hipY + 0.06f;
    float spineBaseY = pelvisY + 0.06f + 0.35f;
    float ribY = spineBaseY + 0.35f + 0.275f;
    float headY = ribY + 0.275f + 0.22f;

    Vector3 root = zombie->position;
    float bodyRot = 0.0f;
    float deathProgress = 0.0f;
    if (zombie->dying) {
        deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
        if (deathProgress > 1.0f) deathProgress = 1.0f;
        bodyRot = deathProgress * 90.0f;
    }

    Vector3 spinePos = (Vector3){ root.x, spineBaseY, root.z };
    if (zombie->dying) spinePos.y -= deathProgress * spineBaseY;
    SetModelTexture(&zombie->spineModel, zombie->boneTex);
    DrawModelEx(zombie->spineModel, spinePos, (Vector3){ 1, 0, 0 }, bodyRot, (Vector3){ 1, 1, 1 }, bodyColor);

    Vector3 ribPos = (Vector3){ root.x, ribY, root.z };
    if (zombie->dying) ribPos.y -= deathProgress * ribY;
    SetModelTexture(&zombie->ribcageModel, zombie->boneTex);
    DrawModelEx(zombie->ribcageModel, ribPos, (Vector3){ 1, 0, 0 }, bodyRot, (Vector3){ 1, 1, 1 }, bodyColor);

    Vector3 pelvisPos = (Vector3){ root.x, pelvisY, root.z };
    if (zombie->dying) pelvisPos.y -= deathProgress * pelvisY;
    SetModelTexture(&zombie->pelvisModel, zombie->boneTex);
    DrawModelEx(zombie->pelvisModel, pelvisPos, (Vector3){ 1, 0, 0 }, bodyRot, (Vector3){ 1, 1, 1 }, bodyColor);

    Vector3 headPos = (Vector3){ root.x, headY, root.z };
    if (zombie->dying) headPos.y -= deathProgress * headY;
    if (!zombie->dying || zombie->type != ZOMBIE_TYPE_IMAGE_HEAD) {
        if (zombie->type == ZOMBIE_TYPE_IMAGE_HEAD && headTextureCount > 0 && zombie->textureIndex < headTextureCount) {
            if (headTextures[zombie->textureIndex].id != 0) {
                SetModelTexture(&zombie->skullModel, headTextures[zombie->textureIndex]);
            } else {
                SetModelTexture(&zombie->skullModel, zombie->boneTex);
            }
        } else {
            SetModelTexture(&zombie->skullModel, zombie->boneTex);
        }
        DrawModelEx(zombie->skullModel, headPos, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, headColor);
    }

    Vector3 shoulderL = (Vector3){ root.x - 0.22f, ribY, root.z };
    Vector3 shoulderR = (Vector3){ root.x + 0.22f, ribY, root.z };
    Vector3 hipL = (Vector3){ root.x - 0.15f, pelvisY, root.z };
    Vector3 hipR = (Vector3){ root.x + 0.15f, pelvisY, root.z };

    float armSwing = walk * 0.5f;
    float legSwing = walk * 0.6f;
    
    if (zombie->dying) {
        float groundY = 0.05f;
        shoulderL.y = shoulderL.y + (groundY - shoulderL.y) * deathProgress;
        shoulderR.y = shoulderR.y + (groundY - shoulderR.y) * deathProgress;
        hipL.y = hipL.y + (groundY - hipL.y) * deathProgress;
        hipR.y = hipR.y + (groundY - hipR.y) * deathProgress;
        armSwing = deathProgress * 2.0f;
        legSwing = deathProgress * 2.0f;
    }

    Vector3 playerPos = camera.position;
    float distToPlayer = Vector3Length(Vector3Subtract(playerPos, zombie->position));
    bool reaching = distToPlayer < REACH_DIST;
    float armAngleX = reaching ? -1.2f : -0.6f;
    float armAngleY = reaching ? 0.3f : -0.8f;

    DrawJoint(shoulderL, 0.04f, bodyColor);
    DrawJoint(shoulderR, 0.04f, bodyColor);

    Vector3 leftElbow = Vector3Add(shoulderL, (Vector3){ armAngleX * 0.55f, armAngleY * 0.55f, 0 });
    Vector3 rightElbow = Vector3Add(shoulderR, (Vector3){ -armAngleX * 0.55f, armAngleY * 0.55f, 0 });
    DrawLimb(zombie->leftUpperArm, shoulderL, (Vector3){ armAngleX, armAngleY, 0 }, armSwing, 0.55f);
    DrawJoint(leftElbow, 0.035f, bodyColor);
    DrawLimb(zombie->leftLowerArm, leftElbow, (Vector3){ armAngleX * 0.7f, armAngleY * 0.8f, 0 }, armSwing * 1.3f, 0.50f);

    DrawLimb(zombie->rightUpperArm, shoulderR, (Vector3){ -armAngleX, armAngleY, 0 }, -armSwing, 0.55f);
    DrawJoint(rightElbow, 0.035f, bodyColor);
    DrawLimb(zombie->rightLowerArm, rightElbow, (Vector3){ -armAngleX * 0.7f, armAngleY * 0.8f, 0 }, -armSwing * 1.3f, 0.50f);

    Vector3 leftWrist = Vector3Add(leftElbow, (Vector3){ armAngleX * 0.7f * 0.50f, armAngleY * 0.8f * 0.50f, 0 });
    Vector3 rightWrist = Vector3Add(rightElbow, (Vector3){ -armAngleX * 0.7f * 0.50f, armAngleY * 0.8f * 0.50f, 0 });
    DrawModelEx(zombie->leftHand, leftWrist, (Vector3){ 0, 1, 0 }, armSwing * 1.3f * RAD2DEG, (Vector3){ 1, 1, 1 }, bodyColor);
    DrawModelEx(zombie->rightHand, rightWrist, (Vector3){ 0, 1, 0 }, -armSwing * 1.3f * RAD2DEG, (Vector3){ 1, 1, 1 }, bodyColor);

    DrawJoint(hipL, 0.045f, bodyColor);
    DrawJoint(hipR, 0.045f, bodyColor);

    Vector3 leftKnee = Vector3Add(hipL, (Vector3){ -0.2f * 0.65f, -1.0f * 0.65f, 0 });
    Vector3 rightKnee = Vector3Add(hipR, (Vector3){ 0.2f * 0.65f, -1.0f * 0.65f, 0 });
    DrawLimb(zombie->leftUpperLeg, hipL, (Vector3){ -0.2f, -1.0f, 0 }, -legSwing, 0.65f);
    DrawJoint(leftKnee, 0.045f, bodyColor);
    DrawLimb(zombie->leftLowerLeg, leftKnee, (Vector3){ -0.15f, -1.0f, 0 }, -legSwing * 1.2f, 0.60f);

    DrawLimb(zombie->rightUpperLeg, hipR, (Vector3){ 0.2f, -1.0f, 0 }, legSwing, 0.65f);
    DrawJoint(rightKnee, 0.045f, bodyColor);
    DrawLimb(zombie->rightLowerLeg, rightKnee, (Vector3){ 0.15f, -1.0f, 0 }, legSwing * 1.2f, 0.60f);

    Vector3 leftAnkle = Vector3Add(leftKnee, (Vector3){ -0.15f * 0.60f, -1.0f * 0.60f, 0 });
    Vector3 rightAnkle = Vector3Add(rightKnee, (Vector3){ 0.15f * 0.60f, -1.0f * 0.60f, 0 });
    DrawModelEx(zombie->leftFoot, leftAnkle, (Vector3){ 0, 1, 0 }, -legSwing * 1.2f * RAD2DEG, (Vector3){ 1, 1, 1 }, bodyColor);
    DrawModelEx(zombie->rightFoot, rightAnkle, (Vector3){ 0, 1, 0 }, legSwing * 1.2f * RAD2DEG, (Vector3){ 1, 1, 1 }, bodyColor);
}

void ZombieShutdown(Zombie *zombie) {
    UnloadModel(zombie->skullModel);
    UnloadModel(zombie->spineModel);
    UnloadModel(zombie->ribcageModel);
    UnloadModel(zombie->pelvisModel);
    UnloadModel(zombie->leftUpperArm);
    UnloadModel(zombie->leftLowerArm);
    UnloadModel(zombie->rightUpperArm);
    UnloadModel(zombie->rightLowerArm);
    UnloadModel(zombie->leftUpperLeg);
    UnloadModel(zombie->leftLowerLeg);
    UnloadModel(zombie->rightUpperLeg);
    UnloadModel(zombie->rightLowerLeg);
    UnloadModel(zombie->leftHand);
    UnloadModel(zombie->rightHand);
    UnloadModel(zombie->leftFoot);
    UnloadModel(zombie->rightFoot);
}

bool ZombieIsAlive(Zombie *zombie) {
    return zombie->active && zombie->health > 0.0f && !zombie->dying;
}

void ZombieTakeDamage(Zombie *zombie, float damage) {
    (void)damage;
    zombie->health -= zombie->maxHealth * 0.25f;
    zombie->damageFlashTimer = 2.0f;
    if (zombie->health <= 0 && !zombie->dying) {
        zombie->dying = true;
        zombie->deathTimer = 3.0f;
    }
}
