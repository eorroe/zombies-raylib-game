#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"
#include "zombie.h"
#include "input.h"

#define WEAPON_DAMAGE 25.0f
#define WEAPON_RANGE 100.0f
#define WEAPON_FIRE_RATE 0.0f
#define SCOPE_FOV 15.0f
#define SCOPE_ZOOM 4.0f
#define MAX_AMMO 30
#define RELOAD_TIME 2.0f

typedef struct {
    bool hit;
    float distance;
    int zombieIndex;
    Vector3 point;
} RayHitInfo;

typedef struct {
    Vector3 position;
    Vector3 direction;
    float cooldown;
    int ammo;
    bool reloading;
    float reloadTimer;
    float recoil;
    bool scopeActive;
    Model model;
    float muzzleFlashTimer;
    float swayTimer;
    Vector2 aimOffset;
} Weapon;

void WeaponInit(Weapon *weapon);
void WeaponUpdate(Weapon *weapon, Vector3 playerPos, InputState *input, float dt);
void WeaponRender(Weapon *weapon, Camera3D camera, float yaw);
void WeaponShoot(Weapon *weapon);
void WeaponReload(Weapon *weapon);
bool WeaponCanShoot(Weapon *weapon);
void WeaponApplyRecoil(Weapon *weapon);
RayHitInfo WeaponRaycast(Weapon *weapon, Camera3D camera, Zombie *zombies, int zombieCount);
void WeaponShutdown(Weapon *weapon);

#endif
