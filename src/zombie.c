#include "zombie.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

#define ZOMBIE_HALF_HEIGHT 0.95f

void ZombieInit(Zombie *zombie, Vector3 position, ZombieType type, int textureIndex) {
    zombie->position = position;
    zombie->velocity = (Vector3){ 0 };
    zombie->health = 50.0f + rand() % 50;
    zombie->maxHealth = zombie->health;
    zombie->type = type;
    zombie->textureIndex = textureIndex;
    zombie->attackCooldown = 0.0f;
    zombie->active = true;
    zombie->model = LoadModelFromMesh(GenMeshCylinder(0.6f, 1.9f, 8));
    zombie->animTime = (float)rand() / RAND_MAX * 6.28f;
}

void ZombieUpdate(Zombie *zombie, Vector3 playerPos, float dt) {
    if (!zombie->active) return;
    zombie->animTime += dt * 3.0f;
    Vector3 dir = Vector3Subtract(playerPos, zombie->position);
    float dist = Vector3Length(dir);
    if (dist > 0.1f) {
        dir = Vector3Normalize(dir);
        zombie->velocity = Vector3Scale(dir, ZOMBIE_SPEED_BASE);
        zombie->position = Vector3Add(zombie->position, Vector3Scale(zombie->velocity, dt));
    }
    if (zombie->attackCooldown > 0) zombie->attackCooldown -= dt;
}

void ZombieRender(Zombie *zombie, Shader shader) {
    if (!zombie->active) return;
    float bob = sinf(zombie->animTime) * 0.1f;
    DrawModelEx(zombie->model, Vector3Add(zombie->position, (Vector3){ 0, ZOMBIE_HALF_HEIGHT + bob, 0 }), (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1.2f, 1 }, (Color){ 80, 100, 70, 255 });
    DrawSphere(Vector3Add(zombie->position, (Vector3){ 0, 1.9f, 0 }), 0.35f, (Color){ 90, 110, 80, 255 });
}

void ZombieShutdown(Zombie *zombie) {
    UnloadModel(zombie->model);
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
