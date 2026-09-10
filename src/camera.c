#include "camera.h"
#include "player.h"
#include "weapon.h"
#include "raymath.h"

void CameraInit(GameCamera *cam, Player *player) {
    cam->camera.position = Vector3Add(player->position, (Vector3){ -4.0f, 3.0f, -6.0f });
    cam->camera.target = Vector3Add(player->position, (Vector3){ 0.0f, 1.8f, 3.0f });
    cam->camera.up = (Vector3){ 0, 1, 0 };
    cam->camera.fovy = 50.0f;
    cam->camera.projection = CAMERA_PERSPECTIVE;
    cam->target = player->position;
    cam->distance = 4.0f;
    cam->height = 2.0f;
    cam->smoothSpeed = 5.0f;
    cam->firstPersonBlend = 0.0f;
    cam->adsBlend = 0.0f;
    cam->isAiming = false;
    cam->mode = GAME_CAMERA_MODE_THIRD_PERSON;
    cam->baseMode = GAME_CAMERA_MODE_THIRD_PERSON;
    cam->crouchAmount = 0.0f;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
}

void CameraUpdate(GameCamera *cam, Player *player, float dt) {
    Vector3 forward = PlayerGetForward(player);
    
    Vector3 tpForward = (Vector3){
        sinf(player->yaw) * cosf(-player->pitch),
        sinf(player->pitch),
        cosf(player->yaw) * cosf(-player->pitch)
    };
    Vector3 desiredPos = Vector3Add(player->position, Vector3Scale(tpForward, -cam->distance));
    desiredPos.y += cam->height;
    
    Vector3 tpTarget = Vector3Add(player->position, (Vector3){ 0, 1.0f, 0 });
    Vector3 fpPos = Vector3Add(player->position, (Vector3){ 0, 1.6f, 0 });
    Vector3 fpForward = (Vector3){
        sinf(player->yaw) * cosf(player->pitch),
        sinf(player->pitch),
        cosf(player->yaw) * cosf(player->pitch)
    };
    Vector3 fpTarget = Vector3Add(fpPos, fpForward);
    
    Vector3 adsOffset = (Vector3){ 0.0f, -0.12f, -0.35f };
    Vector3 adsPos = Vector3Add(fpPos, adsOffset);
    Vector3 adsTarget = Vector3Add(fpTarget, adsOffset);
    
    if (cam->isAiming && cam->adsBlend < 1.0f) {
        cam->adsBlend += dt * 8.0f;
        if (cam->adsBlend > 1.0f) cam->adsBlend = 1.0f;
    } else if (!cam->isAiming && cam->adsBlend > 0.0f) {
        cam->adsBlend -= dt * 8.0f;
        if (cam->adsBlend < 0.0f) cam->adsBlend = 0.0f;
    }
    
    float adsT = cam->adsBlend;
    Vector3 finalFpPos = Vector3Lerp(fpPos, adsPos, adsT);
    Vector3 finalFpTarget = Vector3Lerp(fpTarget, adsTarget, adsT);
    
    float targetBlend = (cam->mode == GAME_CAMERA_MODE_FIRST_PERSON) ? 1.0f : 0.0f;
    float blendSpeed = 4.0f;
    if (targetBlend > cam->firstPersonBlend) {
        cam->firstPersonBlend += dt * blendSpeed;
        if (cam->firstPersonBlend > targetBlend) cam->firstPersonBlend = targetBlend;
    } else if (targetBlend < cam->firstPersonBlend) {
        cam->firstPersonBlend -= dt * blendSpeed;
        if (cam->firstPersonBlend < targetBlend) cam->firstPersonBlend = targetBlend;
    }
    
    float crouchTarget = cam->crouchAmount;
    float crouchSpeed = 8.0f;
    if (crouchTarget > cam->crouchAmount) {
        cam->crouchAmount += dt * crouchSpeed;
        if (cam->crouchAmount > crouchTarget) cam->crouchAmount = crouchTarget;
    } else if (crouchTarget < cam->crouchAmount) {
        cam->crouchAmount -= dt * crouchSpeed;
        if (cam->crouchAmount < crouchTarget) cam->crouchAmount = crouchTarget;
    }
    
    float crouchOffset = cam->crouchAmount * 0.7f;
    desiredPos.y -= crouchOffset;
    finalFpPos.y -= crouchOffset;
    
    float t = cam->firstPersonBlend;
    cam->camera.position = Vector3Lerp(desiredPos, finalFpPos, t);
    cam->camera.target = Vector3Lerp(tpTarget, finalFpTarget, t);
}

void CameraApplyScope(GameCamera *cam, bool active) {
    cam->isAiming = active;
    if (active) {
        cam->camera.fovy = SCOPE_FOV;
    } else {
        cam->camera.fovy = 60.0f;
    }
}

Camera3D CameraGetCamera(GameCamera *cam) {
    return cam->camera;
}

float CameraGetFirstPersonBlend(GameCamera *cam) {
    return cam->firstPersonBlend;
}

float CameraGetADSBlend(GameCamera *cam) {
    return cam->adsBlend;
}

void CameraSetAiming(GameCamera *cam, bool aiming) {
    cam->isAiming = aiming;
}

void CameraToggleMode(GameCamera *cam) {
    cam->baseMode = (cam->baseMode == GAME_CAMERA_MODE_THIRD_PERSON) ? GAME_CAMERA_MODE_FIRST_PERSON : GAME_CAMERA_MODE_THIRD_PERSON;
    cam->mode = cam->baseMode;
}

void CameraSetMode(GameCamera *cam, GameCameraMode mode) {
    cam->mode = mode;
}

GameCameraMode CameraGetMode(GameCamera *cam) {
    return cam->mode;
}

void CameraSetCrouch(GameCamera *cam, bool crouching) {
    cam->crouchAmount = crouching ? 1.0f : 0.0f;
}

float CameraGetCrouchAmount(GameCamera *cam) {
    return cam->crouchAmount;
}