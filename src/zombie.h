#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "raylib.h"
#include "texture.h"

#define MAX_ZOMBIES 64
#define ZOMBIE_SPEED_BASE 2.0f

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
    Model model;
    float animTime;
} Zombie;

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex);
void ZombieUpdate(Zombie *zombie, Vector3 playerPos, float dt);
void ZombieRender(Zombie *zombie, Shader shader);
void ZombieShutdown(Zombie *zombie);
bool ZombieIsAlive(Zombie *zombie);
void ZombieTakeDamage(Zombie *zombie, float damage);

#endif