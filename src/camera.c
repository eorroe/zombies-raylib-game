#include "camera.h"
#include "player.h"
#include "weapon.h"
#include "raymath.h"

void CameraInit(GameCamera *cam, Player *player) {
    cam->camera.position = player->position;
    cam->camera.position.y += 1.7f;
    cam->camera.target = Vector3Add(cam->camera.position, (Vector3){ 0, 0, 1 });
    cam->camera.up = (Vector3){ 0, 1, 0 };
    cam->camera.fovy = 60.0f;
    cam->camera.projection = CAMERA_PERSPECTIVE;
    cam->target = player->position;
    cam->distance = 0.0f;
    cam->height = 1.7f;
    cam->smoothSpeed = 10.0f;
}

void CameraUpdate(GameCamera *cam, Player *player, float dt) {
    Vector3 forward = PlayerGetForward(player);
    Vector3 desiredPos = player->position;
    desiredPos.y += 1.7f;
    cam->camera.position = Vector3Lerp(cam->camera.position, desiredPos, cam->smoothSpeed * dt);
    cam->camera.target = Vector3Add(cam->camera.position, forward);
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
