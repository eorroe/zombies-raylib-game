#include "weapon.h"
#include "raymath.h"
#include "particle.h"
#include "audio.h"

void WeaponInit(Weapon *weapon) {
    weapon->position = (Vector3){ 0.3f, -0.3f, -0.5f };
    weapon->direction = (Vector3){ 0, 0, 1 };
    weapon->cooldown = 0.0f;
    weapon->ammo = MAX_AMMO;
    weapon->reloading = false;
    weapon->reloadTimer = 0.0f;
    weapon->recoil = 0.0f;
    weapon->scopeActive = false;
    weapon->model = LoadModelFromMesh(GenMeshCylinder(0.05f, 0.8f, 8));
    weapon->muzzleFlashTimer = 0.0f;
}

void WeaponUpdate(Weapon *weapon, Camera3D camera, float dt) {
    if (weapon->reloading) {
        weapon->reloadTimer -= dt;
        if (weapon->reloadTimer <= 0) {
            weapon->ammo = MAX_AMMO;
            weapon->reloading = false;
        }
    }
    if (weapon->cooldown > 0) weapon->cooldown -= dt;
    if (weapon->recoil > 0) weapon->recoil -= dt * 2.0f;
    if (weapon->muzzleFlashTimer > 0) weapon->muzzleFlashTimer -= dt;
    
    weapon->position = Vector3Add(camera.position, Vector3Scale(camera.target, 0.5f));
    weapon->position = Vector3Add(weapon->position, (Vector3){ 0.3f, -0.3f, 0 });
    weapon->direction = camera.target;
}

void WeaponRender(Weapon *weapon, Camera3D camera) {
    if (weapon->reloading) return;
    DrawModelEx(weapon->model, weapon->position, (Vector3){ 0, 1, 0 }, 0.0f, (Vector3){ 1, 1, 1 }, DARKGRAY);
    if (weapon->muzzleFlashTimer > 0) {
        DrawSphere(weapon->position, 0.1f, YELLOW);
    }
}

void WeaponShoot(Weapon *weapon) {
    if (!WeaponCanShoot(weapon)) return;
    weapon->cooldown = WEAPON_FIRE_RATE;
    weapon->ammo--;
    weapon->recoil = 1.0f;
    weapon->muzzleFlashTimer = 0.05f;
}

void WeaponReload(Weapon *weapon) {
    if (weapon->reloading || weapon->ammo == MAX_AMMO) return;
    weapon->reloading = true;
    weapon->reloadTimer = RELOAD_TIME;
}

bool WeaponCanShoot(Weapon *weapon) {
    return weapon->cooldown <= 0 && weapon->ammo > 0 && !weapon->reloading;
}

void WeaponApplyRecoil(Weapon *weapon) {
    (void)weapon;
}

RayHitInfo WeaponRaycast(Weapon *weapon, Camera3D camera, Zombie *zombies, int zombieCount) {
    RayHitInfo result = { 0 };
    Ray ray = { camera.position, Vector3Normalize(camera.target) };
    float minDist = WEAPON_RANGE;
    for (int i = 0; i < zombieCount; i++) {
        if (!ZombieIsAlive(&zombies[i])) continue;
        RayCollision col = GetRayCollisionSphere(ray, zombies[i].position, 1.0f);
        if (col.hit && col.distance < minDist) {
            minDist = col.distance;
            result.hit = true;
            result.distance = col.distance;
            result.zombieIndex = i;
            result.point = col.point;
        }
    }
    return result;
}

void WeaponShutdown(Weapon *weapon) {
    UnloadModel(weapon->model);
}
