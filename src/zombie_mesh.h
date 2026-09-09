#ifndef ZOMBIE_MESH_H
#define ZOMBIE_MESH_H

#include "raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

Mesh ZombieMesh_CreateTorso(float width, float height, float depth);
Mesh ZombieMesh_CreateHead(float radius);
Mesh ZombieMesh_CreateJaw(float radius);
Mesh ZombieMesh_CreateLimb(float radius, float length);
Mesh ZombieMesh_CreateHand(float scale);
Mesh ZombieMesh_CreateFoot(float scale);
void ZombieMesh_ComputeNormals(Mesh *mesh);
void ZombieMesh_Upload(Mesh *mesh);
void ZombieMesh_Unload(Mesh *mesh);

#ifdef __cplusplus
}
#endif

#endif
