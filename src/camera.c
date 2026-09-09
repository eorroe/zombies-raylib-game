#include "camera.h"
#include "player.h"
#include "weapon.h"
#include "raymath.h"

void CameraInit(GameCamera *cam, Player *player) {
    cam->camera.position = Vector3Add(player->position, (Vector3){ 0, 2.0f, 4.0f });
    cam->camera.target = Vector3Add(player->position, (Vector3){ 0, 1.0f, 0 });
    cam->camera.up = (Vector3){ 0, 1, 0 };
    cam->camera.fovy = 60.0f;
    cam->camera.projection = CAMERA_PERSPECTIVE;
    cam->target = player->position;
    cam->distance = 4.0f;
    cam->height = 2.0f;
    cam->smoothSpeed = 5.0f;
}

void CameraUpdate(GameCamera *cam, Player *player, float dt) {
    Vector3 forward = PlayerGetForward(player);
    Vector3 desiredPos = Vector3Add(player->position, Vector3Scale(forward, -cam->distance));
    desiredPos.y += cam->height;
    cam->camera.position = Vector3Lerp(cam->camera.position, desiredPos, cam->smoothSpeed * dt);
    cam->camera.target = Vector3Lerp(cam->camera.target, Vector3Add(player->position, (Vector3){ 0, 1.0f, 0 }), cam->smoothSpeed * dt);
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
