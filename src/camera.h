#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"
#include "player.h"

typedef enum {
    GAME_CAMERA_MODE_THIRD_PERSON,
    GAME_CAMERA_MODE_FIRST_PERSON
} GameCameraMode;

typedef struct {
    Camera3D camera;
    Vector3 target;
    float distance;
    float height;
    float smoothSpeed;
    float firstPersonBlend;
    float adsBlend;
    bool isAiming;
    GameCameraMode mode;
    GameCameraMode baseMode;
    float crouchAmount;
    float crouchTarget;
} GameCamera;

void CameraInit(GameCamera *cam, Player *player);
void CameraUpdate(GameCamera *cam, Player *player, float dt);
void CameraApplyScope(GameCamera *cam, bool active);
Camera3D CameraGetCamera(GameCamera *cam);
float CameraGetFirstPersonBlend(GameCamera *cam);
float CameraGetADSBlend(GameCamera *cam);
void CameraSetAiming(GameCamera *cam, bool aiming);
void CameraToggleMode(GameCamera *cam);
void CameraSetMode(GameCamera *cam, GameCameraMode mode);
GameCameraMode CameraGetMode(GameCamera *cam);
void CameraSetCrouch(GameCamera *cam, bool crouching);
float CameraGetCrouchAmount(GameCamera *cam);

#endif
