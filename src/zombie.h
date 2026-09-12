#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "raylib.h"
#include "texture.h"

#define MAX_ZOMBIES 150
#define ZOMBIE_SPEED_BASE 4.5f
#define LIMB_RADIUS 0.13f
#define TORSO_WIDTH 0.60f
#define TORSO_HEIGHT 0.90f
#define HEAD_RADIUS 0.28f
#define ARM_UPPER_LEN 0.70f
#define ARM_LOWER_LEN 0.65f
#define LEG_UPPER_LEN 0.80f
#define LEG_LOWER_LEN 0.75f
#define REACH_DIST 4.0f
#define FENCE_Z 8.5f

typedef enum {
    ZOMBIE_TYPE_DEFAULT,
    ZOMBIE_TYPE_IMAGE_HEAD
} ZombieType;

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float health;
    float maxHealth;
    ZombieType type;
    int textureIndex;
    float attackCooldown;
    bool active;
    float animTime;
    float walkCycle;
    Model bodyModel;
    Model headModel;
    Model jawModel;
    Model leftUpperArm;
    Model leftLowerArm;
    Model rightUpperArm;
    Model rightLowerArm;
    Model leftUpperLeg;
    Model leftLowerLeg;
    Model rightUpperLeg;
    Model rightLowerLeg;
    Model leftHandModel;
    Model rightHandModel;
    Model leftFootModel;
    Model rightFootModel;
    Texture2D skinTex;
    Texture2D skinNormal;
    Texture2D shirtTex;
    Texture2D pantsTex;
    float speed;
    float damageFlashTimer;
    bool dying;
    float deathTimer;
    float torsoWidth;
    float torsoHeight;
    float headRadius;
    float armUpperLen;
    float armLowerLen;
    float legUpperLen;
    float legLowerLen;
    float limbRadius;
    float facingAngle;
    float armSwingOffset;
    float legSwingOffset;
    float armReachOffset;
} Zombie;

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex, Texture2D skin, Texture2D skinNormal, Texture2D shirt, Texture2D pants, Shader pbr);
void ZombieUpdate(Zombie *zombie, Vector3 playerPos, float dt, bool firstShotFired);
void ZombieRender(Zombie *zombie, Camera3D camera, Texture2D *headTextures, int headTextureCount, Shader shader);
void ZombieShutdown(Zombie *zombie);
bool ZombieIsAlive(Zombie *zombie);
void ZombieTakeDamage(Zombie *zombie, float damage);

#endif
