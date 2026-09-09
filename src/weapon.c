#include "weapon.h"
#include "raymath.h"
#include "particle.h"
#include "audio.h"

typedef struct {
    Model body;
    Model barrel;
    Model grip;
    Model magazine;
    Model stock;
    Model sight;
} WeaponModels;

static WeaponModels g_weaponModels;

void WeaponInit(Weapon *weapon) {
    weapon->position = (Vector3){ 0.3f, -0.3f, -0.5f };
    weapon->direction = (Vector3){ 0, 0, 1 };
    weapon->cooldown = 0.0f;
    weapon->ammo = MAX_AMMO;
    weapon->reloading = false;
    weapon->reloadTimer = 0.0f;
    weapon->recoil = 0.0f;
    weapon->scopeActive = false;
    weapon->muzzleFlashTimer = 0.0f;

    Mesh bodyMesh = GenMeshCube(0.06f, 0.08f, 0.22f);
    g_weaponModels.body = LoadModelFromMesh(bodyMesh);

    Mesh barrelMesh = GenMeshCylinder(0.012f, 0.14f, 8);
    g_weaponModels.barrel = LoadModelFromMesh(barrelMesh);

    Mesh gripMesh = GenMeshCube(0.025f, 0.05f, 0.025f);
    g_weaponModels.grip = LoadModelFromMesh(gripMesh);

    Mesh magMesh = GenMeshCube(0.02f, 0.04f, 0.012f);
    g_weaponModels.magazine = LoadModelFromMesh(magMesh);

    Mesh stockMesh = GenMeshCube(0.025f, 0.035f, 0.05f);
    g_weaponModels.stock = LoadModelFromMesh(stockMesh);

    Mesh sightMesh = GenMeshCylinder(0.006f, 0.015f, 8);
    g_weaponModels.sight = LoadModelFromMesh(sightMesh);
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
    
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = camera.up;
    
    weapon->position = Vector3Add(camera.position, Vector3Scale(forward, 0.5f));
    weapon->position = Vector3Add(weapon->position, Vector3Scale(right, 0.3f));
    weapon->position = Vector3Add(weapon->position, Vector3Scale(up, -0.3f));
    weapon->direction = forward;
    
    weapon->swayTimer += dt * 8.0f;
}

void WeaponRender(Weapon *weapon, Camera3D camera) {
    if (weapon->reloading) return;

    float recoilOffset = weapon->recoil * 0.02f;
    Vector3 pos = weapon->position;
    pos = Vector3Add(pos, Vector3Scale(weapon->direction, -recoilOffset));

    Vector3 right = Vector3Normalize(Vector3CrossProduct(weapon->direction, (Vector3){ 0, 1, 0 }));
    Vector3 up = Vector3CrossProduct(right, weapon->direction);

    float swayX = sinf(weapon->swayTimer) * 0.003f;
    float swayY = cosf(weapon->swayTimer * 0.7f) * 0.002f;
    pos = Vector3Add(pos, Vector3Scale(right, swayX));
    pos = Vector3Add(pos, Vector3Scale(up, swayY));

    Vector3 bodyPos = pos;
    DrawModelEx(g_weaponModels.body, bodyPos, up, 0.0f, (Vector3){ 1, 1, 1 }, DARKGRAY);

    Vector3 barrelPos = Vector3Add(pos, Vector3Scale(weapon->direction, 0.1f));
    DrawModelEx(g_weaponModels.barrel, barrelPos, up, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 30, 30, 35, 255 });

    Vector3 gripPos = Vector3Add(pos, (Vector3){ 0, -0.04f, -0.01f });
    DrawModelEx(g_weaponModels.grip, gripPos, up, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 25, 20, 15, 255 });

    Vector3 magPos = Vector3Add(pos, (Vector3){ 0, -0.03f, 0.03f });
    DrawModelEx(g_weaponModels.magazine, magPos, up, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 35, 30, 25, 255 });

    Vector3 stockPos = Vector3Add(pos, (Vector3){ 0, 0, -0.1f });
    DrawModelEx(g_weaponModels.stock, stockPos, up, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 40, 30, 20, 255 });

    Vector3 sightPos = Vector3Add(pos, (Vector3){ 0, 0.04f, 0.02f });
    DrawModelEx(g_weaponModels.sight, sightPos, up, 0.0f, (Vector3){ 1, 1, 1 }, (Color){ 20, 20, 25, 255 });

    if (weapon->muzzleFlashTimer > 0) {
        DrawSphere(barrelPos, 0.03f, YELLOW);
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
    UnloadModel(g_weaponModels.barrel);
    UnloadModel(g_weaponModels.grip);
    UnloadModel(g_weaponModels.magazine);
    UnloadModel(g_weaponModels.stock);
    UnloadModel(g_weaponModels.sight);
}
