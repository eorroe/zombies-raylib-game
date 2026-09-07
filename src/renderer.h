#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "game.h"
#include "zombie.h"
#include "particle.h"

void RendererInit(Game *game, int screenWidth, int screenHeight);
void RendererBegin(Game *game, Camera3D camera);
void RendererDrawScene(Game *game);
void RendererDrawZombies(Game *game, Shader shader);
void RendererDrawPlayer(Player *player, Shader shader);
void RendererDrawParticles(Particle *particles, int count);
void RendererDrawHUD(Game *game);
void RendererDrawScope(Game *game);
void RendererEnd(Game *game);
void RendererShutdown(Game *game);

#endif
