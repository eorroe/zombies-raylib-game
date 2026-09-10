#ifndef ZOMBIE_MESH_H
#define ZOMBIE_MESH_H

#include "raylib.h"

void ZombieMesh_Upload(Mesh *mesh);
void ZombieMesh_Unload(Mesh *mesh);

Mesh ZombieMesh_CreateTorso(float width, float height, float depth);
Mesh ZombieMesh_CreateHead(float radius);
Mesh ZombieMesh_CreateJaw(float radius);
Mesh ZombieMesh_CreateLimb(float radius, float length);
Mesh ZombieMesh_CreateHand(float scale);
Mesh ZombieMesh_CreateFoot(float scale);
Mesh ZombieMesh_CreateSpine(float height);
Mesh ZombieMesh_CreateRibcage(float width, float height);
Mesh ZombieMesh_CreatePelvis(float width, float height);

#endif
