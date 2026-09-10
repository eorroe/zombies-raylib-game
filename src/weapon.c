#include "weapon.h"
#include "raymath.h"
#include "particle.h"
#include "audio.h"
#include "texture.h"
#include "input.h"
#include <stdlib.h>
#include <math.h>

static void SetModelTexture(Model *model, Texture2D tex) {
    if (model->meshCount > 0 && model->materialCount > 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    }
}

typedef struct {
    Model body;
    Model barrel;
    Model barrelRings[4];
    Model grip;
    Model magazine;
    Model stock;
    Model sight;
    Model trigger;
    Model hand;
    Model forearm;
    Model muzzleBrake;
    Model handguard;
    Model triggerGuard;
    Model magRelease;
    Model ejectionPort;
    Model frontSight;
    Model rearSight;
    Texture2D metalTex;
    Texture2D darkMetalTex;
    Texture2D gripTex;
    Texture2D camoTex;
    Texture2D skinTex;
} WeaponModels;

static WeaponModels g_weaponModels;

void WeaponInit(Weapon *weapon, Shader pbr) {
    weapon->position = (Vector3){ 0.25f, 0.55f, 0.15f };
    weapon->direction = (Vector3){ 0, 0, 1 };
    weapon->ammo = MAX_AMMO;
    weapon->reloading = false;
    weapon->reloadTimer = 0.0f;
    weapon->recoil = 0.0f;
    weapon->muzzleFlashTimer = 0.0f;
    weapon->aimOffset = (Vector2){ 0 };

    Mesh bodyMesh = GenMeshCube(0.07f, 0.09f, 0.7f);
    g_weaponModels.body = LoadModelFromMesh(bodyMesh);
    g_weaponModels.body.materials[0].shader = pbr;

    Mesh barrelMesh = GenMeshCylinder(0.022f, 0.42f, 24);
    g_weaponModels.barrel = LoadModelFromMesh(barrelMesh);
    g_weaponModels.barrel.materials[0].shader = pbr;

    for (int i = 0; i < 4; i++) {
        Mesh ringMesh = GenMeshCylinder(0.028f, 0.007f, 24);
        g_weaponModels.barrelRings[i] = LoadModelFromMesh(ringMesh);
        g_weaponModels.barrelRings[i].materials[0].shader = pbr;
    }

    Mesh gripMesh = GenMeshCylinder(0.032f, 0.09f, 12);
    g_weaponModels.grip = LoadModelFromMesh(gripMesh);
    g_weaponModels.grip.materials[0].shader = pbr;

    Mesh magMesh = GenMeshCube(0.038f, 0.08f, 0.028f);
    g_weaponModels.magazine = LoadModelFromMesh(magMesh);
    g_weaponModels.magazine.materials[0].shader = pbr;

    Mesh stockMesh = GenMeshCube(0.05f, 0.065f, 0.15f);
    g_weaponModels.stock = LoadModelFromMesh(stockMesh);
    g_weaponModels.stock.materials[0].shader = pbr;

    Mesh sightMesh = GenMeshCylinder(0.012f, 0.035f, 12);
    g_weaponModels.sight = LoadModelFromMesh(sightMesh);
    g_weaponModels.sight.materials[0].shader = pbr;

    Mesh triggerMesh = GenMeshCube(0.01f, 0.014f, 0.022f);
    g_weaponModels.trigger = LoadModelFromMesh(triggerMesh);
    g_weaponModels.trigger.materials[0].shader = pbr;

    Mesh handMesh = GenMeshCylinder(0.042f, 0.13f, 12);
    g_weaponModels.hand = LoadModelFromMesh(handMesh);
    g_weaponModels.hand.materials[0].shader = pbr;

    Mesh forearmMesh = GenMeshCylinder(0.038f, 0.2f, 12);
    g_weaponModels.forearm = LoadModelFromMesh(forearmMesh);
    g_weaponModels.forearm.materials[0].shader = pbr;

    Mesh muzzleBrakeMesh = GenMeshCylinder(0.03f, 0.022f, 16);
    g_weaponModels.muzzleBrake = LoadModelFromMesh(muzzleBrakeMesh);
    g_weaponModels.muzzleBrake.materials[0].shader = pbr;

    Mesh handguardMesh = GenMeshCube(0.065f, 0.065f, 0.18f);
    g_weaponModels.handguard = LoadModelFromMesh(handguardMesh);
    g_weaponModels.handguard.materials[0].shader = pbr;

    Mesh triggerGuardMesh = GenMeshCylinder(0.007f, 0.022f, 8);
    g_weaponModels.triggerGuard = LoadModelFromMesh(triggerGuardMesh);
    g_weaponModels.triggerGuard.materials[0].shader = pbr;

    Mesh magReleaseMesh = GenMeshCylinder(0.007f, 0.014f, 8);
    g_weaponModels.magRelease = LoadModelFromMesh(magReleaseMesh);
    g_weaponModels.magRelease.materials[0].shader = pbr;

    Mesh ejectionPortMesh = GenMeshCube(0.022f, 0.01f, 0.035f);
    g_weaponModels.ejectionPort = LoadModelFromMesh(ejectionPortMesh);
    g_weaponModels.ejectionPort.materials[0].shader = pbr;

    Mesh frontSightMesh = GenMeshCylinder(0.007f, 0.022f, 8);
    g_weaponModels.frontSight = LoadModelFromMesh(frontSightMesh);
    g_weaponModels.frontSight.materials[0].shader = pbr;

    Mesh rearSightMesh = GenMeshCube(0.018f, 0.018f, 0.025f);
    g_weaponModels.rearSight = LoadModelFromMesh(rearSightMesh);
    g_weaponModels.rearSight.materials[0].shader = pbr;

    Image metalImg = GenImageColor(256, 256, (Color){ 130, 125, 120, 255 });
    for (int i = 0; i < 700; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 100 + rand() % 50;
        ImageDrawPixel(&metalImg, x, y, (Color){ shade, shade - 5, shade - 10, 255 });
    }
    g_weaponModels.metalTex = LoadTextureFromImage(metalImg);
    UnloadImage(metalImg);

    Image darkMetalImg = GenImageColor(256, 256, (Color){ 70, 68, 65, 255 });
    for (int i = 0; i < 500; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 50 + rand() % 30;
        ImageDrawPixel(&darkMetalImg, x, y, (Color){ shade, shade - 3, shade - 5, 255 });
    }
    g_weaponModels.darkMetalTex = LoadTextureFromImage(darkMetalImg);
    UnloadImage(darkMetalImg);

    Image gripImg = GenImageColor(256, 256, (Color){ 100, 78, 60, 255 });
    for (int i = 0; i < 600; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 70 + rand() % 40;
        ImageDrawPixel(&gripImg, x, y, (Color){ shade, shade - 10, shade - 20, 255 });
    }
    g_weaponModels.gripTex = LoadTextureFromImage(gripImg);
    UnloadImage(gripImg);

    Image camoImg = GenImageColor(256, 256, (Color){ 70, 80, 50, 255 });
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            float n = FractalNoise(x, y, 3, 0.5f);
            int idx = (y * 256 + x) * 4;
            int shade = (int)(65 + n * 35);
            if (shade > 255) shade = 255;
            if (shade < 30) shade = 30;
            unsigned char *data = (unsigned char *)camoImg.data;
            data[idx + 0] = (unsigned char)shade;
            data[idx + 1] = (unsigned char)(shade + 8);
            data[idx + 2] = (unsigned char)(shade - 12);
            data[idx + 3] = 255;
        }
    }
    g_weaponModels.camoTex = LoadTextureFromImage(camoImg);
    UnloadImage(camoImg);

    Image skinImg = GenImageColor(256, 256, (Color){ 200, 160, 130, 255 });
    for (int i = 0; i < 800; i++) {
        int x = rand() % 256;
        int y = rand() % 256;
        int shade = 160 + rand() % 60;
        ImageDrawPixel(&skinImg, x, y, (Color){ shade, shade - 20, shade - 50, 255 });
    }
    g_weaponModels.skinTex = LoadTextureFromImage(skinImg);
    UnloadImage(skinImg);

    SetModelTexture(&g_weaponModels.body, g_weaponModels.camoTex);
    SetModelTexture(&g_weaponModels.barrel, g_weaponModels.darkMetalTex);
    SetModelTexture(&g_weaponModels.grip, g_weaponModels.gripTex);
    SetModelTexture(&g_weaponModels.magazine, g_weaponModels.darkMetalTex);
    SetModelTexture(&g_weaponModels.stock, g_weaponModels.camoTex);
    SetModelTexture(&g_weaponModels.sight, g_weaponModels.metalTex);
    SetModelTexture(&g_weaponModels.trigger, g_weaponModels.darkMetalTex);
    for (int i = 0; i < 4; i++) {
        SetModelTexture(&g_weaponModels.barrelRings[i], g_weaponModels.metalTex);
    }
    SetModelTexture(&g_weaponModels.hand, g_weaponModels.skinTex);
    SetModelTexture(&g_weaponModels.forearm, g_weaponModels.camoTex);
    SetModelTexture(&g_weaponModels.muzzleBrake, g_weaponModels.darkMetalTex);
    SetModelTexture(&g_weaponModels.handguard, g_weaponModels.camoTex);
    SetModelTexture(&g_weaponModels.triggerGuard, g_weaponModels.metalTex);
    SetModelTexture(&g_weaponModels.magRelease, g_weaponModels.darkMetalTex);
    SetModelTexture(&g_weaponModels.ejectionPort, g_weaponModels.darkMetalTex);
    SetModelTexture(&g_weaponModels.frontSight, g_weaponModels.metalTex);
    SetModelTexture(&g_weaponModels.rearSight, g_weaponModels.metalTex);
}

static Vector3 RotateOffsetY(Vector3 offset, float cosYaw, float sinYaw) {
    return (Vector3){
        offset.x * cosYaw + offset.z * sinYaw,
        offset.y,
        -offset.x * sinYaw + offset.z * cosYaw
    };
}

void WeaponUpdate(Weapon *weapon, Vector3 playerPos, float yaw, InputState *input, float dt) {
    (void)input;
    if (weapon->reloading) {
        weapon->reloadTimer -= dt;
        if (weapon->reloadTimer <= 0) {
            weapon->ammo = MAX_AMMO;
            weapon->reloading = false;
        }
    }
    if (weapon->recoil > 0) weapon->recoil -= dt * 2.0f;
    if (weapon->muzzleFlashTimer > 0) weapon->muzzleFlashTimer -= dt;
    
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);
    
    float torsoHeight = 0.85f;
    float hipY = playerPos.y + torsoHeight * 0.45f;
    float torsoCenterY = hipY + torsoHeight * 0.5f;
    
    Vector3 torsoOffset = (Vector3){ 0.0f, torsoCenterY, 0.0f };
    Vector3 torsoPos = Vector3Add(playerPos, RotateOffsetY(torsoOffset, cosYaw, sinYaw));
    
    Vector3 shoulderROffset = (Vector3){ 0.55f * 0.6f, torsoCenterY + 0.85f * 0.35f - torsoPos.y, 0.0f };
    Vector3 shoulderR = Vector3Add(torsoPos, RotateOffsetY(shoulderROffset, cosYaw, sinYaw));
    
    Vector3 armOffsetDirRight = RotateOffsetY((Vector3){ 0.6f, -0.8f, 0.0f }, cosYaw, sinYaw);
    Vector3 elbowR = Vector3Add(shoulderR, Vector3Scale(armOffsetDirRight, 0.55f));
    Vector3 wristR = Vector3Add(elbowR, Vector3Scale(armOffsetDirRight, 0.5f));
    
    weapon->position = wristR;
    weapon->direction = (Vector3){ 0, 0, 1 };
    
    weapon->swayTimer += dt * 8.0f;
}

static void DrawDetailedGun(Vector3 pos, float yawDeg, Color tint) {
    Vector3 right = (Vector3){ 1, 0, 0 };
    Vector3 up = (Vector3){ 0, 1, 0 };

    Vector3 bodyPos = pos;
    DrawModelEx(g_weaponModels.body, bodyPos, up, yawDeg, (Vector3){ 1, 1, 1 }, tint);

    Vector3 barrelPos = Vector3Add(pos, (Vector3){ 0, 0, 0.22f });
    DrawModelEx(g_weaponModels.barrel, barrelPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 45, 48, 52, 255 });

    for (int i = 0; i < 4; i++) {
        float t = (float)i / 3.0f;
        Vector3 ringPos = Vector3Add(barrelPos, (Vector3){ 0, 0, t * 0.16f });
        DrawModelEx(g_weaponModels.barrelRings[i], ringPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 70, 72, 78, 255 });
    }

    Vector3 muzzlePos = Vector3Add(barrelPos, (Vector3){ 0, 0, 0.42f });
    DrawModelEx(g_weaponModels.muzzleBrake, muzzlePos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 35, 38, 42, 255 });

    Vector3 scopePos = Vector3Add(bodyPos, (Vector3){ 0, 0.09f, 0.02f });
    DrawModelEx(g_weaponModels.sight, scopePos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 55, 58, 65, 255 });

    Vector3 handguardPos = Vector3Add(pos, (Vector3){ 0, 0, 0.06f });
    DrawModelEx(g_weaponModels.handguard, handguardPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 60, 62, 70, 255 });

    Vector3 frontSightPos = Vector3Add(handguardPos, (Vector3){ 0, 0.04f, 0.07f });
    DrawModelEx(g_weaponModels.frontSight, frontSightPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 40, 42, 48, 255 });

    Vector3 gripPos = Vector3Add(pos, (Vector3){ 0, -0.07f, -0.015f });
    DrawModelEx(g_weaponModels.grip, gripPos, (Vector3){ 1, 0, 0 }, 0.15f + yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 120, 90, 60, 255 });

    Vector3 triggerGuardPos = Vector3Add(gripPos, (Vector3){ 0, -0.025f, 0.01f });
    DrawModelEx(g_weaponModels.triggerGuard, triggerGuardPos, (Vector3){ 0, 0, 1 }, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 60, 62, 70, 255 });

    Vector3 triggerPos = Vector3Add(pos, (Vector3){ 0, -0.02f, 0.01f });
    DrawModelEx(g_weaponModels.trigger, triggerPos, (Vector3){ 1, 0, 0 }, 0.0f + yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 45, 48, 55, 255 });

    Vector3 magPos = Vector3Add(pos, (Vector3){ 0, -0.06f, 0.04f });
    DrawModelEx(g_weaponModels.magazine, magPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 50, 52, 60, 255 });

    Vector3 magReleasePos = Vector3Add(magPos, (Vector3){ 0.02f, 0.0f, 0.0f });
    DrawModelEx(g_weaponModels.magRelease, magReleasePos, (Vector3){ 1, 0, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 70, 72, 80, 255 });

    Vector3 ejectionPortPos = Vector3Add(bodyPos, (Vector3){ 0.04f, 0.01f, 0.1f });
    DrawModelEx(g_weaponModels.ejectionPort, ejectionPortPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 40, 42, 48, 255 });

    Vector3 stockPos = Vector3Add(pos, (Vector3){ 0, 0, -0.18f });
    DrawModelEx(g_weaponModels.stock, stockPos, up, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 100, 80, 55, 255 });

    Vector3 handPos = Vector3Add(pos, (Vector3){ 0, -0.04f, 0.12f });
    DrawModelEx(g_weaponModels.hand, handPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 200, 160, 130, 255 });

    Vector3 forearmPos = Vector3Add(pos, (Vector3){ 0, -0.04f, 0.22f });
    DrawModelEx(g_weaponModels.forearm, forearmPos, (Vector3){ 0, 1, 0 }, yawDeg, (Vector3){ 1, 1, 1 }, (Color){ 70, 80, 50, 255 });
}

void WeaponRender(Weapon *weapon, Camera3D camera, float yaw) {
    if (weapon->reloading) return;

    float recoilOffset = weapon->recoil * 0.02f;
    Vector3 pos = weapon->position;
    pos.z -= recoilOffset;

    Vector3 right = (Vector3){ 1, 0, 0 };
    Vector3 up = (Vector3){ 0, 1, 0 };

    float swayX = sinf(weapon->swayTimer) * 0.003f;
    float swayY = cosf(weapon->swayTimer * 0.7f) * 0.002f;
    pos = Vector3Add(pos, Vector3Scale(right, swayX));
    pos = Vector3Add(pos, Vector3Scale(up, swayY));

    float yawDeg = yaw * RAD2DEG;
    DrawDetailedGun(pos, yawDeg, (Color){ 70, 85, 170, 255 });

    if (weapon->muzzleFlashTimer > 0) {
        Vector3 barrelTip = Vector3Add(pos, (Vector3){ 0, 0, 0.7f });
        DrawSphere(barrelTip, 0.1f, YELLOW);
    }
}

void WeaponRenderFirstPerson(Weapon *weapon, Camera3D camera, float yaw) {
    if (weapon->reloading) return;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

    float recoilOffset = weapon->recoil * 0.02f;
    Vector3 basePos = Vector3Add(camera.position, Vector3Scale(forward, 0.5f));
    basePos = Vector3Add(basePos, Vector3Scale(right, 0.25f));
    basePos = Vector3Add(basePos, Vector3Scale(up, -0.2f));
    basePos.z -= recoilOffset;

    float swayX = sinf(weapon->swayTimer) * 0.003f;
    float swayY = cosf(weapon->swayTimer * 0.7f) * 0.002f;
    Vector3 pos = Vector3Add(basePos, Vector3Scale(right, swayX));
    pos = Vector3Add(pos, Vector3Scale(up, swayY));

    float yawDeg = yaw * RAD2DEG;
    DrawDetailedGun(pos, yawDeg, (Color){ 70, 85, 170, 255 });

    if (weapon->muzzleFlashTimer > 0) {
        Vector3 barrelTip = Vector3Add(pos, (Vector3){ 0, 0, 0.7f });
        DrawSphere(barrelTip, 0.1f, YELLOW);
    }
}

void WeaponShoot(Weapon *weapon) {
    if (!WeaponCanShoot(weapon)) return;
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
    return weapon->ammo > 0 && !weapon->reloading;
}

void WeaponApplyRecoil(Weapon *weapon) {
    (void)weapon;
}

RayHitInfo WeaponRaycast(Weapon *weapon, Camera3D camera, Zombie *zombies, int zombieCount) {
    RayHitInfo result = { 0 };
    Vector3 dir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Ray ray = { camera.position, dir };
    float minDist = 10000.0f;
    for (int i = 0; i < zombieCount; i++) {
        if (!ZombieIsAlive(&zombies[i])) continue;
        Vector3 zombieCenter = Vector3Add(zombies[i].position, (Vector3){ 0, 1.0f, 0 });
        RayCollision col = GetRayCollisionSphere(ray, zombieCenter, 1.5f);
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
    (void)weapon;
    UnloadModel(g_weaponModels.body);
    UnloadModel(g_weaponModels.barrel);
    UnloadModel(g_weaponModels.grip);
    UnloadModel(g_weaponModels.magazine);
    UnloadModel(g_weaponModels.stock);
    UnloadModel(g_weaponModels.sight);
    UnloadModel(g_weaponModels.trigger);
    UnloadModel(g_weaponModels.hand);
    UnloadModel(g_weaponModels.forearm);
    UnloadModel(g_weaponModels.muzzleBrake);
    UnloadModel(g_weaponModels.handguard);
    UnloadModel(g_weaponModels.triggerGuard);
    UnloadModel(g_weaponModels.magRelease);
    UnloadModel(g_weaponModels.ejectionPort);
    UnloadModel(g_weaponModels.frontSight);
    UnloadModel(g_weaponModels.rearSight);
    for (int i = 0; i < 4; i++) {
        UnloadModel(g_weaponModels.barrelRings[i]);
    }
}
