#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "input.h"

#define PLAYER_SPEED 5.0f
#define PLAYER_HEALTH 100.0f
#define PLAYER_HEIGHT 1.8f
#define PLAYER_RADIUS 0.5f
#define PLAYER_LEG_LENGTH 0.9f

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float health;
    float maxHealth;
    float yaw;
    float pitch;
    Model bodyModel;
    Model headModel;
    Model leftArmModel;
    Model rightArmModel;
    Model leftLegModel;
    Model rightLegModel;
    Texture2D uniformTex;
    Texture2D skinTex;
    bool isMoving;
    float footstepTimer;
    float animTime;
    Vector3 moveDir;
} Player;

void PlayerInit(Player *player, Vector3 startPos);
void PlayerUpdate(Player *player, InputState *input, float dt);
void PlayerRender(Player *player, Shader shader);
void PlayerShutdown(Player *player);
void PlayerTakeDamage(Player *player, float damage);
Vector3 PlayerGetForward(Player *player);
Vector3 PlayerGetRight(Player *player);

#endif
