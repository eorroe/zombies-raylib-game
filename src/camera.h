#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"
#include "player.h"

typedef struct {
    Camera3D camera;
    Vector3 target;
    float distance;
    float height;
    float smoothSpeed;
    float firstPersonBlend;
    float adsBlend;
    bool isAiming;
} GameCamera;

void CameraInit(GameCamera *cam, Player *player);
void CameraUpdate(GameCamera *cam, Player *player, float dt);
void CameraApplyScope(GameCamera *cam, bool active);
Camera3D CameraGetCamera(GameCamera *cam);
float CameraGetFirstPersonBlend(GameCamera *cam);
float CameraGetADSBlend(GameCamera *cam);
void CameraSetAiming(GameCamera *cam, bool aiming);

#endif