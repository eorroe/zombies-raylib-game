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

static void SetModelNormal(Model *model, Texture2D normal) {
    if (model->meshCount > 0 && model->materialCount > 0 && normal.id != 0) {
        model->materials[0].maps[MATERIAL_MAP_NORMAL].texture = normal;
    }
}

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex, Texture2D skin, Texture2D skinNormal, Texture2D shirt, Texture2D pants) {
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
    zombie->skinNormal = skinNormal;
    zombie->shirtTex = shirt;
    zombie->pantsTex = pants;
    zombie->speed = ZOMBIE_SPEED_BASE;
    zombie->damageFlashTimer = 0.0f;

    float torsoW = TORSO_WIDTH * (0.8f + (rand() % 100) / 250.0f);
    float torsoH = TORSO_HEIGHT * (0.85f + (rand() % 100) / 300.0f);
    float headR = HEAD_RADIUS * (0.85f + (rand() % 100) / 250.0f);
    float armUpper = ARM_UPPER_LEN * (0.85f + (rand() % 100) / 300.0f);
    float armLower = ARM_LOWER_LEN * (0.85f + (rand() % 100) / 300.0f);
    float legUpper = LEG_UPPER_LEN * (0.85f + (rand() % 100) / 300.0f);
    float legLower = LEG_LOWER_LEN * (0.85f + (rand() % 100) / 300.0f);
    float limbR = LIMB_RADIUS * (0.8f + (rand() % 100) / 250.0f);

    zombie->torsoWidth = torsoW;
    zombie->torsoHeight = torsoH;
    zombie->headRadius = headR;
    zombie->armUpperLen = armUpper;
    zombie->armLowerLen = armLower;
    zombie->legUpperLen = legUpper;
    zombie->legLowerLen = legLower;
    zombie->limbRadius = limbR;

    Mesh torsoMesh = GenMeshCylinder(torsoW, torsoH, 12);
    zombie->bodyModel = LoadModelFromMesh(torsoMesh);
    SetModelTexture(&zombie->bodyModel, shirt);
    SetModelNormal(&zombie->bodyModel, skinNormal);

    Mesh headMesh = GenMeshSphere(headR, 16, 16);
    zombie->headModel = LoadModelFromMesh(headMesh);
    SetModelTexture(&zombie->headModel, skin);
    SetModelNormal(&zombie->headModel, skinNormal);

    Mesh jawMesh = GenMeshCylinder(headR * 0.7f, headR * 0.5f, 8);
    zombie->jawModel = LoadModelFromMesh(jawMesh);
    SetModelTexture(&zombie->jawModel, skin);
    SetModelNormal(&zombie->jawModel, skinNormal);

    zombie->leftUpperArm = CreateLimbMesh(limbR, armUpper, 10);
    SetModelTexture(&zombie->leftUpperArm, shirt);
    SetModelNormal(&zombie->leftUpperArm, skinNormal);
    zombie->leftLowerArm = CreateLimbMesh(limbR * 0.8f, armLower, 10);
    SetModelTexture(&zombie->leftLowerArm, skin);
    SetModelNormal(&zombie->leftLowerArm, skinNormal);
    zombie->rightUpperArm = CreateLimbMesh(limbR, armUpper, 10);
    SetModelTexture(&zombie->rightUpperArm, shirt);
    SetModelNormal(&zombie->rightUpperArm, skinNormal);
    zombie->rightLowerArm = CreateLimbMesh(limbR * 0.8f, armLower, 10);
    SetModelTexture(&zombie->rightLowerArm, skin);
    SetModelNormal(&zombie->rightLowerArm, skinNormal);

    zombie->leftUpperLeg = CreateLimbMesh(limbR * 1.1f, legUpper, 10);
    SetModelTexture(&zombie->leftUpperLeg, pants);
    SetModelNormal(&zombie->leftUpperLeg, skinNormal);
    zombie->leftLowerLeg = CreateLimbMesh(limbR * 0.9f, legLower, 10);
    SetModelTexture(&zombie->leftLowerLeg, pants);
    SetModelNormal(&zombie->leftLowerLeg, skinNormal);
    zombie->rightUpperLeg = CreateLimbMesh(limbR * 1.1f, legUpper, 10);
    SetModelTexture(&zombie->rightUpperLeg, pants);
    SetModelNormal(&zombie->rightUpperLeg, skinNormal);
    zombie->rightLowerLeg = CreateLimbMesh(limbR * 0.9f, legLower, 10);
    SetModelTexture(&zombie->rightLowerLeg, pants);
    SetModelNormal(&zombie->rightLowerLeg, skinNormal);
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

static void DrawLimb(Model model, Vector3 origin, Vector3 axis, float angle, float length) {
    Vector3 mid = Vector3Add(origin, Vector3Scale(axis, length * 0.5f));
    DrawModelEx(model, mid, (Vector3){ 0, 0, 1 }, angle * RAD2DEG, (Vector3){ 1, 1, 1 }, WHITE);
}

void ZombieRender(Zombie *zombie, Camera3D camera, Texture2D *headTextures, int headTextureCount, Shader shader) {
    if (!zombie->active) return;
    float bob = sinf(zombie->animTime) * 0.05f;
    float walk = sinf(zombie->walkCycle);
    float feetY = zombie->position.y + bob;

    float flash = 0.0f;
    if (zombie->damageFlashTimer > 0.0f) {
        float cycle = fmodf(zombie->damageFlashTimer, 1.0f);
        if (cycle < 0.5f) flash = 1.0f;
    }
    
    Color skinColor = (Color){ 255, 100, 100, 255 };
    Color shirtColor = (Color){ 255, 30, 30, 255 };
    Color pantsColor = (Color){ 30, 30, 255, 255 };
    Color flashRed = (Color){ 255, 0, 0, 255 };
    
    Color bodyColor = (Color){
        (unsigned char)(shirtColor.r + (flashRed.r - shirtColor.r) * flash),
        (unsigned char)(shirtColor.g + (flashRed.g - shirtColor.g) * flash),
        (unsigned char)(shirtColor.b + (flashRed.b - shirtColor.b) * flash),
        255
    };
    Color headColor = (Color){
        (unsigned char)(skinColor.r + (flashRed.r - skinColor.r) * flash),
        (unsigned char)(skinColor.g + (flashRed.g - skinColor.g) * flash),
        (unsigned char)(skinColor.b + (flashRed.b - skinColor.b) * flash),
        255
    };
    
    float hipY = feetY + zombie->legUpperLen + zombie->legLowerLen;
    float torsoCenterY = hipY + zombie->torsoHeight * 0.5f;
    float headCenterY = hipY + zombie->torsoHeight + zombie->headRadius * 0.9f;

    Vector3 torsoPos = (Vector3){ zombie->position.x, torsoCenterY, zombie->position.z };
    float bodyRot = 0.0f;
    float bodyY = torsoPos.y;
    
    if (zombie->dying) {
        float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
        if (deathProgress > 1.0f) deathProgress = 1.0f;
        bodyRot = deathProgress * 110.0f;
        bodyY = torsoPos.y - deathProgress * torsoPos.y * 0.8f;
    }
    
    SetModelTexture(&zombie->bodyModel, zombie->shirtTex);
    DrawModelEx(zombie->bodyModel, (Vector3){ torsoPos.x, bodyY, torsoPos.z }, (Vector3){ 1, 0, 0 }, bodyRot, (Vector3){ 1, 1, 1 }, bodyColor);

    if (!zombie->dying || zombie->type != ZOMBIE_TYPE_IMAGE_HEAD) {
        Vector3 headPos = (Vector3){ zombie->position.x, headCenterY, zombie->position.z };
        float headY = headPos.y;
        if (zombie->dying) {
            float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
            if (deathProgress > 1.0f) deathProgress = 1.0f;
            headY = headPos.y - deathProgress * headPos.y;
        }
        if (zombie->type == ZOMBIE_TYPE_IMAGE_HEAD && headTextureCount > 0 && zombie->textureIndex < headTextureCount) {
            if (headTextures[zombie->textureIndex].id != 0) {
                SetModelTexture(&zombie->headModel, headTextures[zombie->textureIndex]);
            } else {
                SetModelTexture(&zombie->headModel, zombie->skinTex);
            }
        } else {
            SetModelTexture(&zombie->headModel, zombie->skinTex);
        }
        DrawModelEx(zombie->headModel, (Vector3){ headPos.x, headY, headPos.z }, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, headColor);
        if (!zombie->dying || zombie->type != ZOMBIE_TYPE_IMAGE_HEAD) {
            Vector3 jawPos = (Vector3){ headPos.x, headY - zombie->headRadius * 0.3f, headPos.z + zombie->headRadius * 0.4f };
            float jawY = jawPos.y;
            if (zombie->dying) {
                float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
                if (deathProgress > 1.0f) deathProgress = 1.0f;
                jawY = jawPos.y - deathProgress * jawPos.y;
            }
            DrawModelEx(zombie->jawModel, (Vector3){ jawPos.x, jawY, jawPos.z }, (Vector3){ 1, 0, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, headColor);
        }
    }

    Vector3 shoulderL = (Vector3){ zombie->position.x - zombie->torsoWidth * 0.6f, torsoCenterY + zombie->torsoHeight * 0.35f, zombie->position.z };
    Vector3 shoulderR = (Vector3){ zombie->position.x + zombie->torsoWidth * 0.6f, torsoCenterY + zombie->torsoHeight * 0.35f, zombie->position.z };
    Vector3 hipL = (Vector3){ zombie->position.x - zombie->torsoWidth * 0.35f, hipY, zombie->position.z };
    Vector3 hipR = (Vector3){ zombie->position.x + zombie->torsoWidth * 0.35f, hipY, zombie->position.z };

    float armSwing = walk * 0.5f;
    float legSwing = walk * 0.6f;
    
    if (zombie->dying) {
        float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
        if (deathProgress > 1.0f) deathProgress = 1.0f;
        armSwing = deathProgress * 3.0f;
        legSwing = deathProgress * 2.5f;
        float groundY = 0.05f;
        shoulderL.y = shoulderL.y + (groundY - shoulderL.y) * deathProgress;
        shoulderR.y = shoulderR.y + (groundY - shoulderR.y) * deathProgress;
        hipL.y = hipL.y + (groundY - hipL.y) * deathProgress;
        hipR.y = hipR.y + (groundY - hipR.y) * deathProgress;
    }

    Vector3 playerPos = camera.position;
    float distToPlayer = Vector3Length(Vector3Subtract(playerPos, zombie->position));
    bool reaching = distToPlayer < REACH_DIST;
    float armAngleX = reaching ? -1.2f : -0.6f;
    float armAngleY = reaching ? 0.3f : -0.8f;

    DrawLimb(zombie->leftUpperArm, shoulderL, (Vector3){ armAngleX, armAngleY, 0 }, armSwing, zombie->armUpperLen);
    DrawLimb(zombie->leftLowerArm, Vector3Add(shoulderL, (Vector3){ armAngleX * zombie->armUpperLen, armAngleY * zombie->armUpperLen, 0 }), (Vector3){ armAngleX * 0.7f, armAngleY * 0.8f, 0 }, armSwing * 1.3f, zombie->armLowerLen);

    DrawLimb(zombie->rightUpperArm, shoulderR, (Vector3){ -armAngleX, armAngleY, 0 }, -armSwing, zombie->armUpperLen);
    DrawLimb(zombie->rightLowerArm, Vector3Add(shoulderR, (Vector3){ -armAngleX * zombie->armUpperLen, armAngleY * zombie->armUpperLen, 0 }), (Vector3){ -armAngleX * 0.7f, armAngleY * 0.8f, 0 }, -armSwing * 1.3f, zombie->armLowerLen);

    DrawLimb(zombie->leftUpperLeg, hipL, (Vector3){ -0.2f, -1.0f, 0 }, -legSwing, zombie->legUpperLen);
    DrawLimb(zombie->leftLowerLeg, Vector3Add(hipL, (Vector3){ -0.2f * zombie->legUpperLen, -1.0f * zombie->legUpperLen, 0 }), (Vector3){ -0.15f, -1.0f, 0 }, -legSwing * 1.2f, zombie->legLowerLen);

    DrawLimb(zombie->rightUpperLeg, hipR, (Vector3){ 0.2f, -1.0f, 0 }, legSwing, zombie->legUpperLen);
    DrawLimb(zombie->rightLowerLeg, Vector3Add(hipR, (Vector3){ 0.2f * zombie->legUpperLen, -1.0f * zombie->legUpperLen, 0 }), (Vector3){ 0.15f, -1.0f, 0 }, legSwing * 1.2f, zombie->legLowerLen);
}

void ZombieShutdown(Zombie *zombie) {
    UnloadModel(zombie->bodyModel);
    UnloadModel(zombie->headModel);
    UnloadModel(zombie->jawModel);
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
