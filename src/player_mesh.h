#ifndef PLAYER_MESH_H
#define PLAYER_MESH_H

#include "raylib.h"

Mesh PlayerMesh_CreateTorso(float width, float height, float depth);
Mesh PlayerMesh_CreateHead(float radius);
Mesh PlayerMesh_CreateHelmet(float radius);
Mesh PlayerMesh_CreateLimb(float radius, float length, int slices);
Mesh PlayerMesh_CreateHand(float radius);
Mesh PlayerMesh_CreateFoot(float radius);
void PlayerMesh_ComputeNormals(Mesh *mesh);
void PlayerMesh_Upload(Mesh *mesh);

#endif