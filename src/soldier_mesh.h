#ifndef SOLDIER_MESH_H
#define SOLDIER_MESH_H

#include "raylib.h"

Mesh SoldierMesh_CreateTorso(float width, float height, float depth);
Mesh SoldierMesh_CreateHead(float radius);
Mesh SoldierMesh_CreateHelmet(float radius);
Mesh SoldierMesh_CreateLimb(float radius, float length, int slices);
Mesh SoldierMesh_CreateHand(float radius);
Mesh SoldierMesh_CreateFoot(float radius);
void SoldierMesh_ComputeNormals(Mesh *mesh);
void SoldierMesh_Upload(Mesh *mesh);

#endif