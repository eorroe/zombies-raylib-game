#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "raylib.h"
#include "texture.h"

#define MAX_ZOMBIES 64
#define ZOMBIE_SPEED_BASE 2.0f
#define LIMB_RADIUS 0.09f
#define TORSO_WIDTH 0.45f
#define TORSO_HEIGHT 0.75f
#define HEAD_RADIUS 0.22f
#define ARM_UPPER_LEN 0.55f
#define ARM_LOWER_LEN 0.50f
#define LEG_UPPER_LEN 0.60f
#define LEG_LOWER_LEN 0.60f
#define REACH_DIST 4.0f

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
    Model leftUpperArm;
    Model leftLowerArm;
    Model rightUpperArm;
    Model rightLowerArm;
    Model leftUpperLeg;
    Model leftLowerLeg;
    Model rightUpperLeg;
    Model rightLowerLeg;
    Texture2D skinTex;
    Texture2D shirtTex;
    Texture2D pantsTex;
} Zombie;

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex, Texture2D skin, Texture2D shirt, Texture2D pants);
void ZombieUpdate(Zombie *zombie, Vector3 playerPos, float dt);
void ZombieRender(Zombie *zombie, Camera3D camera, Texture2D *headTextures, int headTextureCount, Shader shader);
void ZombieShutdown(Zombie *zombie);
bool ZombieIsAlive(Zombie *zombie);
void ZombieTakeDamage(Zombie *zombie, float damage);

#endif
