#include "particle.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

void ParticleInit(Particle *p) {
    p->active = false;
    p->position = (Vector3){ 0 };
    p->velocity = (Vector3){ 0 };
    p->life = 0;
    p->maxLife = 0;
    p->type = PARTICLE_BLOOD;
    p->size = 0;
    p->color = WHITE;
}

void ParticleSpawn(Particle *p, Vector3 pos, Vector3 vel, float life, ParticleType type, float size, Color color) {
    p->position = pos;
    p->velocity = vel;
    p->life = life;
    p->maxLife = life;
    p->type = type;
    p->size = size;
    p->color = color;
    p->active = true;
}

void ParticleUpdate(Particle *p, float dt) {
    if (!p->active) return;
    p->life -= dt;
    if (p->life <= 0) {
        p->active = false;
        return;
    }
    p->velocity.y -= 9.8f * dt;
    p->position = Vector3Add(p->position, Vector3Scale(p->velocity, dt));
}

void ParticleRender(Particle *p) {
    if (!p->active) return;
    float alpha = p->life / p->maxLife;
    Color c = p->color;
    c.a = (unsigned char)(alpha * 255);
    DrawSphere(p->position, p->size * alpha, c);
}

bool ParticleIsAlive(Particle *p) {
    return p->active;
}

void ParticleSystemUpdate(Particle *particles, int count, float dt) {
    for (int i = 0; i < count; i++) {
        ParticleUpdate(&particles[i], dt);
    }
}

void ParticleSystemRender(Particle *particles, int count) {
    for (int i = 0; i < count; i++) {
        ParticleRender(&particles[i]);
    }
}
