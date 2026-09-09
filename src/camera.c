#include "camera.h"
#include "player.h"
#include "weapon.h"
#include "raymath.h"

void CameraInit(GameCamera *cam, Player *player) {
    cam->camera.position = Vector3Add(player->position, (Vector3){ 0, 2.0f, -4.0f });
    cam->camera.target = Vector3Add(player->position, (Vector3){ 0, 1.0f, 0 });
    cam->camera.up = (Vector3){ 0, 1, 0 };
    cam->camera.fovy = 60.0f;
    cam->camera.projection = CAMERA_PERSPECTIVE;
    cam->target = player->position;
    cam->distance = 4.0f;
    cam->height = 2.0f;
    cam->smoothSpeed = 8.0f;
    cam->firstPersonBlend = 0.0f;
    cam->isAiming = false;
}

void CameraUpdate(GameCamera *cam, Player *player, float dt) {
    Vector3 forward = PlayerGetForward(player);
    
    Vector3 tpPos = Vector3Add(player->position, Vector3Scale(forward, -cam->distance));
    tpPos.y += cam->height;
    Vector3 tpTarget = Vector3Add(player->position, (Vector3){ 0, 1.0f, 0 });
    
    Vector3 fpPos = Vector3Add(player->position, (Vector3){ 0, 1.6f, 0 });
    Vector3 fpTarget = Vector3Add(player->position, forward);
    
    if (cam->isAiming && cam->firstPersonBlend < 1.0f) {
        cam->firstPersonBlend += dt * 6.0f;
        if (cam->firstPersonBlend > 1.0f) cam->firstPersonBlend = 1.0f;
    } else if (!cam->isAiming && cam->firstPersonBlend > 0.0f) {
        cam->firstPersonBlend -= dt * 6.0f;
        if (cam->firstPersonBlend < 0.0f) cam->firstPersonBlend = 0.0f;
    }
    
    float t = cam->firstPersonBlend;
    cam->camera.position = Vector3Lerp(tpPos, fpPos, t);
    cam->camera.target = Vector3Lerp(tpTarget, fpTarget, t);
}

void CameraApplyScope(GameCamera *cam, bool active) {
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

void CameraSetAiming(GameCamera *cam, bool aiming) {
    cam->isAiming = aiming;
}