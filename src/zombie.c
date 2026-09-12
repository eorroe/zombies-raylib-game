#include "zombie.h"
#include "zombie_mesh.h"
#include "raymath.h"
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

static void SetModelNormal(Model *model, Texture2D normal) {
    if (model->meshCount > 0 && model->materialCount > 0 && normal.id != 0) {
        model->materials[0].maps[MATERIAL_MAP_NORMAL].texture = normal;
    }
}

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex, Texture2D skin, Texture2D skinNormal, Texture2D shirt, Texture2D pants, Shader pbr) {
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
    zombie->facingAngle = 0.0f;
    zombie->armSwingOffset = (rand() % 100) / 500.0f;
    zombie->legSwingOffset = (rand() % 100) / 500.0f;
    zombie->armReachOffset = (rand() % 100) / 500.0f;

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

    Mesh torsoMesh = ZombieMesh_CreateTorso(torsoW, torsoH, torsoH * 0.7f);
    zombie->bodyModel = LoadHighPolyModel(torsoMesh);
    SetModelTexture(&zombie->bodyModel, shirt);
    SetModelNormal(&zombie->bodyModel, skinNormal);
    zombie->bodyModel.materials[0].shader = pbr;

    Mesh headMesh = ZombieMesh_CreateHead(headR);
    zombie->headModel = LoadHighPolyModel(headMesh);
    SetModelTexture(&zombie->headModel, skin);
    SetModelNormal(&zombie->headModel, skinNormal);
    zombie->headModel.materials[0].shader = pbr;

    Mesh jawMesh = ZombieMesh_CreateJaw(headR);
    zombie->jawModel = LoadHighPolyModel(jawMesh);
    SetModelTexture(&zombie->jawModel, skin);
    SetModelNormal(&zombie->jawModel, skinNormal);
    zombie->jawModel.materials[0].shader = pbr;

    Mesh leftUpperArmMesh = ZombieMesh_CreateLimb(limbR, armUpper);
    zombie->leftUpperArm = LoadHighPolyModel(leftUpperArmMesh);
    SetModelTexture(&zombie->leftUpperArm, shirt);
    SetModelNormal(&zombie->leftUpperArm, skinNormal);
    zombie->leftUpperArm.materials[0].shader = pbr;

    Mesh leftLowerArmMesh = ZombieMesh_CreateLimb(limbR * 0.8f, armLower);
    zombie->leftLowerArm = LoadHighPolyModel(leftLowerArmMesh);
    SetModelTexture(&zombie->leftLowerArm, skin);
    SetModelNormal(&zombie->leftLowerArm, skinNormal);
    zombie->leftLowerArm.materials[0].shader = pbr;

    Mesh rightUpperArmMesh = ZombieMesh_CreateLimb(limbR, armUpper);
    zombie->rightUpperArm = LoadHighPolyModel(rightUpperArmMesh);
    SetModelTexture(&zombie->rightUpperArm, shirt);
    SetModelNormal(&zombie->rightUpperArm, skinNormal);
    zombie->rightUpperArm.materials[0].shader = pbr;

    Mesh rightLowerArmMesh = ZombieMesh_CreateLimb(limbR * 0.8f, armLower);
    zombie->rightLowerArm = LoadHighPolyModel(rightLowerArmMesh);
    SetModelTexture(&zombie->rightLowerArm, skin);
    SetModelNormal(&zombie->rightLowerArm, skinNormal);
    zombie->rightLowerArm.materials[0].shader = pbr;

    Mesh leftUpperLegMesh = ZombieMesh_CreateLimb(limbR * 1.1f, legUpper);
    zombie->leftUpperLeg = LoadHighPolyModel(leftUpperLegMesh);
    SetModelTexture(&zombie->leftUpperLeg, pants);
    SetModelNormal(&zombie->leftUpperLeg, skinNormal);
    zombie->leftUpperLeg.materials[0].shader = pbr;

    Mesh leftLowerLegMesh = ZombieMesh_CreateLimb(limbR * 0.9f, legLower);
    zombie->leftLowerLeg = LoadHighPolyModel(leftLowerLegMesh);
    SetModelTexture(&zombie->leftLowerLeg, pants);
    SetModelNormal(&zombie->leftLowerLeg, skinNormal);
    zombie->leftLowerLeg.materials[0].shader = pbr;

    Mesh rightUpperLegMesh = ZombieMesh_CreateLimb(limbR * 1.1f, legUpper);
    zombie->rightUpperLeg = LoadHighPolyModel(rightUpperLegMesh);
    SetModelTexture(&zombie->rightUpperLeg, pants);
    SetModelNormal(&zombie->rightUpperLeg, skinNormal);
    zombie->rightUpperLeg.materials[0].shader = pbr;

    Mesh rightLowerLegMesh = ZombieMesh_CreateLimb(limbR * 0.9f, legLower);
    zombie->rightLowerLeg = LoadHighPolyModel(rightLowerLegMesh);
    SetModelTexture(&zombie->rightLowerLeg, pants);
    SetModelNormal(&zombie->rightLowerLeg, skinNormal);
    zombie->rightLowerLeg.materials[0].shader = pbr;

    Mesh leftHandMesh = ZombieMesh_CreateHand(limbR * 6.0f);
    zombie->leftHandModel = LoadHighPolyModel(leftHandMesh);
    SetModelTexture(&zombie->leftHandModel, skin);
    SetModelNormal(&zombie->leftHandModel, skinNormal);
    zombie->leftHandModel.materials[0].shader = pbr;

    Mesh rightHandMesh = ZombieMesh_CreateHand(limbR * 6.0f);
    zombie->rightHandModel = LoadHighPolyModel(rightHandMesh);
    SetModelTexture(&zombie->rightHandModel, skin);
    SetModelNormal(&zombie->rightHandModel, skinNormal);
    zombie->rightHandModel.materials[0].shader = pbr;

    Mesh leftFootMesh = ZombieMesh_CreateFoot(limbR * 5.0f);
    zombie->leftFootModel = LoadHighPolyModel(leftFootMesh);
    SetModelTexture(&zombie->leftFootModel, pants);
    SetModelNormal(&zombie->leftFootModel, skinNormal);
    zombie->leftFootModel.materials[0].shader = pbr;

    Mesh rightFootMesh = ZombieMesh_CreateFoot(limbR * 5.0f);
    zombie->rightFootModel = LoadHighPolyModel(rightFootMesh);
    SetModelTexture(&zombie->rightFootModel, pants);
    SetModelNormal(&zombie->rightFootModel, skinNormal);
    zombie->rightFootModel.materials[0].shader = pbr;
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
            float targetAngle = atan2f(dir.x, dir.z);
            float diff = targetAngle - zombie->facingAngle;
            while (diff > PI) diff -= 2.0f * PI;
            while (diff < -PI) diff += 2.0f * PI;
            zombie->facingAngle += diff * 5.0f * dt;
            while (zombie->facingAngle > PI) zombie->facingAngle -= 2.0f * PI;
            while (zombie->facingAngle < -PI) zombie->facingAngle += 2.0f * PI;
        }
        
        if (fabsf(zombie->position.z - FENCE_Z) < 0.8f && zombie->position.z < FENCE_Z + 0.5f) {
            zombie->position.z += dt * 1.5f;
            zombie->position.y += dt * 0.8f;
            if (zombie->position.y > 1.2f) zombie->position.y = 1.2f;
            if (zombie->position.z > FENCE_Z + 0.5f) zombie->position.z = FENCE_Z + 0.5f;
        }
    }
    if (zombie->attackCooldown > 0) zombie->attackCooldown -= dt;
    if (zombie->damageFlashTimer > 0.0f) zombie->damageFlashTimer -= dt;
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
    
    DrawCylinderEx(start, end, radius, radius, 6, color);
    DrawSphere(start, radius * 1.2f, color);
    DrawSphere(end, radius * 1.2f, color);
}

static Vector3 RotateY(Vector3 v, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return (Vector3){ v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
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
    
    Color skinColor = (Color){ 70, 110, 160, 255 };
    Color shirtColor = (Color){ 40, 70, 130, 255 };
    Color pantsColor = (Color){ 30, 50, 100, 255 };
    Color flashRed = (Color){ 120, 40, 40, 255 };
    
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
    float bodyY = torsoPos.y;
    
    bool climbing = fabsf(zombie->position.z - FENCE_Z) < 1.0f && zombie->position.z > FENCE_Z - 0.3f && !zombie->dying;
    float climbReach = climbing ? sinf(zombie->animTime * 2.0f) * 0.3f : 0.0f;
    
    float yawDeg = zombie->facingAngle * RAD2DEG;
    
    if (zombie->dying) {
        float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
        if (deathProgress > 1.0f) deathProgress = 1.0f;
        bodyY = torsoPos.y - deathProgress * torsoPos.y * 0.8f;
    }
    Vector3 headPos = (Vector3){ zombie->position.x, headCenterY, zombie->position.z };
    float headY = headPos.y;
    if (zombie->dying) {
        float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
        if (deathProgress > 1.0f) deathProgress = 1.0f;
        headY = headPos.y - deathProgress * headPos.y;
        bodyY = torsoPos.y - deathProgress * torsoPos.y * 0.8f;
    }
    
    Vector3 headFinal = (Vector3){ headPos.x, headY, headPos.z };
    Vector3 torsoFinal = (Vector3){ torsoPos.x, bodyY, torsoPos.z };
    
    // Draw head
    DrawSphere(headFinal, zombie->headRadius * 0.5f, headColor);
    
    // Draw torso as lines between shoulders and hips
    Vector3 shoulderL = Vector3Add(torsoFinal, RotateY((Vector3){ -zombie->torsoWidth * 0.6f, zombie->torsoHeight * 0.35f, 0 }, zombie->facingAngle));
    Vector3 shoulderR = Vector3Add(torsoFinal, RotateY((Vector3){ zombie->torsoWidth * 0.6f, zombie->torsoHeight * 0.35f, 0 }, zombie->facingAngle));
    Vector3 hipL = Vector3Add((Vector3){ zombie->position.x, hipY, zombie->position.z }, RotateY((Vector3){ -zombie->torsoWidth * 0.35f, 0, 0 }, zombie->facingAngle));
    Vector3 hipR = Vector3Add((Vector3){ zombie->position.x, hipY, zombie->position.z }, RotateY((Vector3){ zombie->torsoWidth * 0.35f, 0, 0 }, zombie->facingAngle));
    
    DrawStickLimb(shoulderL, hipL, 0.04f, shirtColor);
    DrawStickLimb(shoulderR, hipR, 0.04f, shirtColor);
    DrawStickLimb(shoulderL, shoulderR, 0.03f, shirtColor);
    
    // Draw neck
    Vector3 neckPos = Vector3Add(headFinal, (Vector3){ 0, -zombie->headRadius * 0.5f, 0 });
    DrawStickLimb(neckPos, shoulderL, 0.02f, skinColor);
    DrawStickLimb(neckPos, shoulderR, 0.02f, skinColor);

    Vector3 shoulderLOffset = (Vector3){ -zombie->torsoWidth * 0.6f, zombie->torsoHeight * 0.35f, 0 };
    Vector3 shoulderROffset = (Vector3){ zombie->torsoWidth * 0.6f, zombie->torsoHeight * 0.35f, 0 };
    Vector3 hipLOffset = (Vector3){ -zombie->torsoWidth * 0.35f, 0, 0 };
    Vector3 hipROffset = (Vector3){ zombie->torsoWidth * 0.35f, 0, 0 };
    
    Vector3 animShoulderL = Vector3Add(torsoPos, RotateY(shoulderLOffset, zombie->facingAngle));
    Vector3 animShoulderR = Vector3Add(torsoPos, RotateY(shoulderROffset, zombie->facingAngle));
    Vector3 animHipL = Vector3Add((Vector3){ zombie->position.x, hipY, zombie->position.z }, RotateY(hipLOffset, zombie->facingAngle));
    Vector3 animHipR = Vector3Add((Vector3){ zombie->position.x, hipY, zombie->position.z }, RotateY(hipROffset, zombie->facingAngle));

    float armSwing = walk * (0.5f + zombie->armSwingOffset);
    float legSwing = walk * (0.6f + zombie->legSwingOffset);
    
    if (zombie->dying) {
        float deathProgress = 1.0f - (zombie->deathTimer / 3.0f);
        if (deathProgress > 1.0f) deathProgress = 1.0f;
        armSwing = deathProgress * 3.0f;
        legSwing = deathProgress * 2.5f;
        float groundY = 0.05f;
        animShoulderL.y = animShoulderL.y + (groundY - animShoulderL.y) * deathProgress;
        animShoulderR.y = animShoulderR.y + (groundY - animShoulderR.y) * deathProgress;
        animHipL.y = animHipL.y + (groundY - animHipL.y) * deathProgress;
        animHipR.y = animHipR.y + (groundY - animHipR.y) * deathProgress;
    }

    Vector3 playerPos = camera.position;
    float distToPlayer = Vector3Length(Vector3Subtract(playerPos, zombie->position));
    bool reaching = distToPlayer < REACH_DIST;
    float armAngleX = climbing ? -1.5f + climbReach : (reaching ? -1.2f + zombie->armReachOffset : -0.6f + zombie->armReachOffset);
    float armAngleY = climbing ? 0.8f : (reaching ? 0.3f : -0.8f);
    
    Vector3 armDirL = RotateY((Vector3){ armAngleX, armAngleY, 0 }, zombie->facingAngle);
    Vector3 armDirR = RotateY((Vector3){ -armAngleX, armAngleY, 0 }, zombie->facingAngle);
    Vector3 legDirL = RotateY((Vector3){ -0.2f, -1.0f, 0 }, zombie->facingAngle);
    Vector3 legDirR = RotateY((Vector3){ 0.2f, -1.0f, 0 }, zombie->facingAngle);

    DrawStickLimb(shoulderL, Vector3Add(shoulderL, Vector3Scale(armDirL, zombie->armUpperLen)), 0.025f, shirtColor);
    DrawStickLimb(Vector3Add(shoulderL, Vector3Scale(armDirL, zombie->armUpperLen)), Vector3Add(shoulderL, Vector3Scale(armDirL, zombie->armUpperLen + zombie->armLowerLen)), 0.02f, skinColor);

    DrawStickLimb(shoulderR, Vector3Add(shoulderR, Vector3Scale(armDirR, zombie->armUpperLen)), 0.025f, shirtColor);
    DrawStickLimb(Vector3Add(shoulderR, Vector3Scale(armDirR, zombie->armUpperLen)), Vector3Add(shoulderR, Vector3Scale(armDirR, zombie->armUpperLen + zombie->armLowerLen)), 0.02f, skinColor);

    DrawStickLimb(hipL, Vector3Add(hipL, Vector3Scale(legDirL, zombie->legUpperLen)), 0.03f, pantsColor);
    DrawStickLimb(Vector3Add(hipL, Vector3Scale(legDirL, zombie->legUpperLen)), Vector3Add(hipL, Vector3Scale(legDirL, zombie->legUpperLen + zombie->legLowerLen)), 0.025f, pantsColor);

    DrawStickLimb(hipR, Vector3Add(hipR, Vector3Scale(legDirR, zombie->legUpperLen)), 0.03f, pantsColor);
    DrawStickLimb(Vector3Add(hipR, Vector3Scale(legDirR, zombie->legUpperLen)), Vector3Add(hipR, Vector3Scale(legDirR, zombie->legUpperLen + zombie->legLowerLen)), 0.025f, pantsColor);

    Vector3 leftHandPos = Vector3Add(shoulderL, Vector3Add(Vector3Scale(armDirL, zombie->armUpperLen), (Vector3){ 0, 0, climbReach * 0.5f }));
    Vector3 rightHandPos = Vector3Add(shoulderR, Vector3Add(Vector3Scale(armDirR, zombie->armUpperLen), (Vector3){ 0, 0, climbReach * 0.5f }));
    DrawSphere(leftHandPos, 0.04f, headColor);
    DrawSphere(rightHandPos, 0.04f, headColor);

    Vector3 leftFootPos = Vector3Add(hipL, Vector3Add(Vector3Scale(legDirL, zombie->legUpperLen), Vector3Scale(legDirL, zombie->legLowerLen * 0.5f)));
    Vector3 rightFootPos = Vector3Add(hipR, Vector3Add(Vector3Scale(legDirR, zombie->legUpperLen), Vector3Scale(legDirR, zombie->legLowerLen * 0.5f)));
    DrawSphere(leftFootPos, 0.035f, pantsColor);
    DrawSphere(rightFootPos, 0.035f, pantsColor);
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
    UnloadModel(zombie->leftHandModel);
    UnloadModel(zombie->rightHandModel);
    UnloadModel(zombie->leftFootModel);
    UnloadModel(zombie->rightFootModel);
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
