#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"

#define MAX_PARTICLES 512

typedef enum {
    PARTICLE_BLOOD,
    PARTICLE_MUZZLE_FLASH,
    PARTICLE_SMOKE,
    PARTICLE_DEBRIS,
    PARTICLE_SPARK
} ParticleType;

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float life;
    float maxLife;
    ParticleType type;
    float size;
    Color color;
    bool active;
} Particle;

void ParticleInit(Particle *p);
void ParticleSpawn(Particle *p, Vector3 pos, Vector3 vel, float life, ParticleType type, float size, Color color);
void ParticleUpdate(Particle *p, float dt);
void ParticleRender(Particle *p);
bool ParticleIsAlive(Particle *p);
void ParticleSystemUpdate(Particle *particles, int count, float dt);
void ParticleSystemRender(Particle *particles, int count);

#endif