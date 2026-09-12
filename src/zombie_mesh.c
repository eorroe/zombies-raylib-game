#include "zombie_mesh.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <raymath.h>

#define PI 3.14159265358979323846f

static void ComputeMeshNormals(Mesh *mesh) {
    if (mesh->vertices == NULL || mesh->normals == NULL || mesh->indices == NULL) return;
    memset(mesh->normals, 0, mesh->vertexCount * 3 * sizeof(float));
    for (int t = 0; t < mesh->triangleCount; t++) {
        int i0 = mesh->indices[t * 3];
        int i1 = mesh->indices[t * 3 + 1];
        int i2 = mesh->indices[t * 3 + 2];
        Vector3 v0 = { mesh->vertices[i0*3], mesh->vertices[i0*3+1], mesh->vertices[i0*3+2] };
        Vector3 v1 = { mesh->vertices[i1*3], mesh->vertices[i1*3+1], mesh->vertices[i1*3+2] };
        Vector3 v2 = { mesh->vertices[i2*3], mesh->vertices[i2*3+1], mesh->vertices[i2*3+2] };
        Vector3 edge1 = Vector3Subtract(v1, v0);
        Vector3 edge2 = Vector3Subtract(v2, v0);
        Vector3 normal = Vector3CrossProduct(edge1, edge2);
        mesh->normals[i0*3] += normal.x; mesh->normals[i0*3+1] += normal.y; mesh->normals[i0*3+2] += normal.z;
        mesh->normals[i1*3] += normal.x; mesh->normals[i1*3+1] += normal.y; mesh->normals[i1*3+2] += normal.z;
        mesh->normals[i2*3] += normal.x; mesh->normals[i2*3+1] += normal.y; mesh->normals[i2*3+2] += normal.z;
    }
    for (int i = 0; i < mesh->vertexCount; i++) {
        Vector3 n = { mesh->normals[i*3], mesh->normals[i*3+1], mesh->normals[i*3+2] };
        float len = Vector3Length(n);
        if (len > 0.0001f) n = Vector3Scale(n, 1.0f / len);
        else n = (Vector3){0,1,0};
        mesh->normals[i*3] = n.x; mesh->normals[i*3+1] = n.y; mesh->normals[i*3+2] = n.z;
    }
}

void ZombieMesh_Upload(Mesh *mesh) {
    if (mesh->vertices != NULL) UploadMesh(mesh, false);
}

void ZombieMesh_Unload(Mesh *mesh) {
    if (mesh->vertices) { free(mesh->vertices); mesh->vertices = NULL; }
    if (mesh->texcoords) { free(mesh->texcoords); mesh->texcoords = NULL; }
    if (mesh->normals) { free(mesh->normals); mesh->normals = NULL; }
    if (mesh->colors) { free(mesh->colors); mesh->colors = NULL; }
    if (mesh->indices) { free(mesh->indices); mesh->indices = NULL; }
    mesh->vertexCount = 0;
    mesh->triangleCount = 0;
}

static float TorsoRadius(float ny, float theta, float width, float height, float depth) {
    float y = ny * height * 0.5f;
    float baseR = 0.0f;

    if (y > height * 0.35f) {
        float t = (y - height * 0.35f) / (height * 0.15f);
        baseR = width * (0.45f + 0.15f * t);
    } else if (y > height * 0.1f) {
        float t = (y - height * 0.1f) / (height * 0.25f);
        baseR = width * (0.55f - 0.1f * t);
    } else if (y > -height * 0.2f) {
        float t = (y - (-height * 0.2f)) / (height * 0.3f);
        baseR = width * (0.45f + 0.1f * t);
    } else {
        float t = (y - (-height * 0.5f)) / (height * 0.3f);
        baseR = width * (0.42f + 0.03f * t);
    }

    float angleMod = 1.0f;
    float ct = cosf(theta);
    if (y > height * 0.15f) {
        float chestT = (y - height * 0.15f) / (height * 0.35f);
        angleMod += 0.08f * chestT * fabsf(ct);
    }
    if (y > -height * 0.15f && y < height * 0.05f) {
        float waistT = 1.0f - fabsf(y - (-height * 0.05f)) / (height * 0.2f);
        angleMod -= 0.06f * waistT * fabsf(ct);
    }
    if (y < -height * 0.2f) {
        float hipT = (-height * 0.2f - y) / (height * 0.3f);
        angleMod += 0.05f * hipT * fabsf(ct);
    }
    angleMod += 0.02f * sinf(theta * 3.0f + ny * 5.0f);
    angleMod += 0.015f * sinf(theta * 7.0f - ny * 3.0f);
    return baseR * angleMod;
}

Mesh ZombieMesh_CreateTorso(float width, float height, float depth) {
    int radialSegs = 96;
    int heightSegs = 48;
    int ringVerts = radialSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = radialSegs * heightSegs;
    int triangleCount = quadCount * 2;
    int topCenter = bodyVerts;
    int bottomCenter = bodyVerts + 1;
    int topRingStart = bottomCenter + 1;
    int bottomRingStart = topRingStart + radialSegs;
    int vertexCount = bottomRingStart + radialSegs;
    int capTris = radialSegs * 2;
    triangleCount += capTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    int vi = 0;
    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float ny = v * 2.0f - 1.0f;
        float y = ny * height * 0.5f;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float r = TorsoRadius(ny, theta, width, height, depth);
            float ct = cosf(theta);
            float st = sinf(theta);

            float detail = 0.0f;
            detail += 0.008f * sinf(theta * 8.0f + ny * 6.0f) * (1.0f - fabsf(ny));
            detail += 0.006f * sinf(theta * 12.0f - ny * 4.0f);
            detail += 0.004f * sinf(theta * 16.0f + ny * 8.0f);
            if (fabsf(ny) < 0.2f && fabsf(ct) > 0.7f) {
                detail += 0.012f * sinf(ny * 20.0f) * (fabsf(ct) - 0.7f) * 3.33f;
            }

            float x = r * ct + detail * ct;
            float z = r * st + detail * st;
            y += detail * 0.5f;

            mesh.vertices[vi * 3] = x;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = z;
            mesh.texcoords[vi * 2] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4] = 200;
            mesh.colors[vi * 4 + 1] = 180;
            mesh.colors[vi * 4 + 2] = 170;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    mesh.vertices[topCenter * 3] = 0;
    mesh.vertices[topCenter * 3 + 1] = height * 0.5f;
    mesh.vertices[topCenter * 3 + 2] = 0;
    mesh.texcoords[topCenter * 2] = 0.5f;
    mesh.texcoords[topCenter * 2 + 1] = 1.0f;
    mesh.colors[topCenter * 4] = 200; mesh.colors[topCenter * 4 + 1] = 180;
    mesh.colors[topCenter * 4 + 2] = 170; mesh.colors[topCenter * 4 + 3] = 255;

    mesh.vertices[bottomCenter * 3] = 0;
    mesh.vertices[bottomCenter * 3 + 1] = -height * 0.5f;
    mesh.vertices[bottomCenter * 3 + 2] = 0;
    mesh.texcoords[bottomCenter * 2] = 0.5f;
    mesh.texcoords[bottomCenter * 2 + 1] = 0.0f;
    mesh.colors[bottomCenter * 4] = 200; mesh.colors[bottomCenter * 4 + 1] = 180;
    mesh.colors[bottomCenter * 4 + 2] = 170; mesh.colors[bottomCenter * 4 + 3] = 255;

    for (int i = 0; i < radialSegs; i++) {
        float u = (float)i / radialSegs;
        float theta = u * 2.0f * PI;
        float r = 0.45f * width;
        int idx = topRingStart + i;
        mesh.vertices[idx * 3] = r * cosf(theta);
        mesh.vertices[idx * 3 + 1] = height * 0.5f;
        mesh.vertices[idx * 3 + 2] = r * sinf(theta);
        mesh.texcoords[idx * 2] = u;
        mesh.texcoords[idx * 2 + 1] = 1.0f;
        mesh.colors[idx * 4] = 200; mesh.colors[idx * 4 + 1] = 180;
        mesh.colors[idx * 4 + 2] = 170; mesh.colors[idx * 4 + 3] = 255;

        r = 0.42f * width;
        idx = bottomRingStart + i;
        mesh.vertices[idx * 3] = r * cosf(theta);
        mesh.vertices[idx * 3 + 1] = -height * 0.5f;
        mesh.vertices[idx * 3 + 2] = r * sinf(theta);
        mesh.texcoords[idx * 2] = u;
        mesh.texcoords[idx * 2 + 1] = 0.0f;
        mesh.colors[idx * 4] = 200; mesh.colors[idx * 4 + 1] = 180;
        mesh.colors[idx * 4 + 2] = 170; mesh.colors[idx * 4 + 3] = 255;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }

    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = topCenter;
        mesh.indices[ii++] = topRingStart + ((i + 1) % radialSegs);
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = bottomCenter;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + ((i + 1) % radialSegs);
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateHead(float radius) {
    int widthSegs = 96;
    int heightSegs = 96;
    int ringVerts = widthSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = widthSegs * heightSegs;
    int triangleCount = quadCount * 2;

    Mesh mesh = { 0 };
    mesh.vertexCount = bodyVerts;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(bodyVerts * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(bodyVerts * 2, sizeof(float));
    mesh.normals = (float *)calloc(bodyVerts * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(bodyVerts * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float phi = v * PI;
        float sp = sinf(phi);
        float cp = cosf(phi);

        for (int i = 0; i <= widthSegs; i++) {
            float u = (float)i / widthSegs;
            float theta = u * 2.0f * PI;
            float ct = cosf(theta);
            float st = sinf(theta);

            float x = radius * sp * ct;
            float y = radius * cp;
            float z = radius * sp * st;

            float displace = 0.0f;
            displace += 0.03f * sinf(theta * 6.0f) * sinf(phi * 8.0f);
            displace += 0.02f * sinf(theta * 10.0f - phi * 4.0f);
            displace += 0.015f * sinf(theta * 14.0f + phi * 6.0f);

            Vector3 eyeL = { -radius * 0.35f, radius * 0.1f, radius * 0.85f };
            Vector3 eyeR = { radius * 0.35f, radius * 0.1f, radius * 0.85f };
            float eyeR_r = radius * 0.18f;
            float dL = sqrtf((x-eyeL.x)*(x-eyeL.x) + (y-eyeL.y)*(y-eyeL.y) + (z-eyeL.z)*(z-eyeL.z));
            float dR = sqrtf((x-eyeR.x)*(x-eyeR.x) + (y-eyeR.y)*(y-eyeR.y) + (z-eyeR.z)*(z-eyeR.z));
            if (dL < eyeR_r) displace -= (eyeR_r - dL) * 1.5f;
            if (dR < eyeR_r) displace -= (eyeR_r - dR) * 1.5f;

            Vector3 nosePos = {0, -radius*0.15f, radius*0.95f};
            float noseR = radius * 0.12f;
            float dN = sqrtf((x-nosePos.x)*(x-nosePos.x) + (y-nosePos.y)*(y-nosePos.y) + (z-nosePos.z)*(z-nosePos.z));
            if (dN < noseR) displace += (noseR - dN) * 1.0f;

            Vector3 mouthPos = {0, -radius*0.4f, radius*0.8f};
            float mouthR = radius * 0.2f;
            float dM = sqrtf((x-mouthPos.x)*(x-mouthPos.x) + (y-mouthPos.y)*(y-mouthPos.y) + (z-mouthPos.z)*(z-mouthPos.z));
            if (dM < mouthR && z > radius * 0.6f) displace -= (mouthR - dM) * 0.8f;

            Vector3 browPos = {0, radius*0.35f, radius*0.75f};
            float browR = radius * 0.25f;
            float dB = sqrtf((x-browPos.x)*(x-browPos.x) + (y-browPos.y)*(y-browPos.y) + (z-browPos.z)*(z-browPos.z));
            if (dB < browR) displace += (browR - dB) * 0.5f;

            Vector3 earL = {-radius*0.9f, 0, 0};
            Vector3 earR = {radius*0.9f, 0, 0};
            float earR_r = radius * 0.15f;
            float dEL = sqrtf((x-earL.x)*(x-earL.x) + (y-earL.y)*(y-earL.y) + (z-earL.z)*(z-earL.z));
            float dER = sqrtf((x-earR.x)*(x-earR.x) + (y-earR.y)*(y-earR.y) + (z-earR.z)*(z-earR.z));
            if (dEL < earR_r && z > 0) displace += (earR_r - dEL) * 0.8f;
            if (dER < earR_r && z > 0) displace += (earR_r - dER) * 0.8f;

            Vector3 jawPos = {0, -radius*0.65f, radius*0.3f};
            float jawR = radius * 0.3f;
            float dJ = sqrtf((x-jawPos.x)*(x-jawPos.x) + (y-jawPos.y)*(y-jawPos.y) + (z-jawPos.z)*(z-jawPos.z));
            if (dJ < jawR && z > 0) displace += (jawR - dJ) * 0.5f;

            Vector3 len = {x, y, z};
            float lenLen = Vector3Length(len);
            if (lenLen > 0.0001f) len = Vector3Scale(len, 1.0f / lenLen);
            x += len.x * displace;
            y += len.y * displace;
            z += len.z * displace;

            int idx = j * ringVerts + i;
            mesh.vertices[idx * 3] = x;
            mesh.vertices[idx * 3 + 1] = y;
            mesh.vertices[idx * 3 + 2] = z;
            mesh.texcoords[idx * 2] = u;
            mesh.texcoords[idx * 2 + 1] = v;
            mesh.colors[idx * 4] = 190;
            mesh.colors[idx * 4 + 1] = 160;
            mesh.colors[idx * 4 + 2] = 150;
            mesh.colors[idx * 4 + 3] = 255;
        }
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < widthSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateJaw(float radius) {
    int radialSegs = 32;
    int heightSegs = 16;
    int ringVerts = radialSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = radialSegs * heightSegs;
    int triangleCount = quadCount * 2;

    Mesh mesh = { 0 };
    mesh.vertexCount = bodyVerts;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(bodyVerts * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(bodyVerts * 2, sizeof(float));
    mesh.normals = (float *)calloc(bodyVerts * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(bodyVerts * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float ny = v * 2.0f - 1.0f;
        float y = ny * radius * 0.6f;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float angleFromFront = fabsf(theta);
            if (angleFromFront > PI) angleFromFront = 2.0f * PI - angleFromFront;

            float r = radius * (0.55f + 0.15f * (1.0f - angleFromFront / PI));
            r *= (1.0f - 0.2f * v);
            float x = r * cosf(theta);
            float z = radius * 0.35f + r * sinf(theta) * 0.5f;
            float detail = 0.01f * sinf(theta * 8.0f) * sinf(v * 6.0f);
            x += detail * cosf(theta);
            z += detail * sinf(theta);

            int idx = j * ringVerts + i;
            mesh.vertices[idx * 3] = x;
            mesh.vertices[idx * 3 + 1] = y;
            mesh.vertices[idx * 3 + 2] = z;
            mesh.texcoords[idx * 2] = u;
            mesh.texcoords[idx * 2 + 1] = v;
            mesh.colors[idx * 4] = 190;
            mesh.colors[idx * 4 + 1] = 160;
            mesh.colors[idx * 4 + 2] = 150;
            mesh.colors[idx * 4 + 3] = 255;
        }
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateLimb(float radius, float length) {
    int radialSegs = 48;
    int heightSegs = 16;
    int ringVerts = radialSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = radialSegs * heightSegs;
    int triangleCount = quadCount * 2;
    int topCenter = bodyVerts;
    int bottomCenter = bodyVerts + 1;
    int topRingStart = bottomCenter + 1;
    int bottomRingStart = topRingStart + radialSegs;
    int vertexCount = bottomRingStart + radialSegs;
    int capTris = radialSegs * 2;
    triangleCount += capTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    int vi = 0;
    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float y = v * length - length * 0.5f;
        float t = v;
        float r = radius * (1.0f - 0.15f * t);
        r += 0.015f * sinf(t * 8.0f) * radius;
        r *= (1.0f + 0.08f * sinf(t * PI));

        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float x = r * cosf(theta);
            float z = r * sinf(theta);
            mesh.vertices[vi * 3] = x;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = z;
            mesh.texcoords[vi * 2] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4] = 190;
            mesh.colors[vi * 4 + 1] = 160;
            mesh.colors[vi * 4 + 2] = 150;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    mesh.vertices[topCenter * 3] = 0;
    mesh.vertices[topCenter * 3 + 1] = length * 0.5f;
    mesh.vertices[topCenter * 3 + 2] = 0;
    mesh.texcoords[topCenter * 2] = 0.5f;
    mesh.texcoords[topCenter * 2 + 1] = 1.0f;
    mesh.colors[topCenter * 4] = 190; mesh.colors[topCenter * 4 + 1] = 160;
    mesh.colors[topCenter * 4 + 2] = 150; mesh.colors[topCenter * 4 + 3] = 255;

    mesh.vertices[bottomCenter * 3] = 0;
    mesh.vertices[bottomCenter * 3 + 1] = -length * 0.5f;
    mesh.vertices[bottomCenter * 3 + 2] = 0;
    mesh.texcoords[bottomCenter * 2] = 0.5f;
    mesh.texcoords[bottomCenter * 2 + 1] = 0.0f;
    mesh.colors[bottomCenter * 4] = 190; mesh.colors[bottomCenter * 4 + 1] = 160;
    mesh.colors[bottomCenter * 4 + 2] = 150; mesh.colors[bottomCenter * 4 + 3] = 255;

    for (int i = 0; i < radialSegs; i++) {
        float u = (float)i / radialSegs;
        float theta = u * 2.0f * PI;
        float r = radius * 1.1f;
        int idx = topRingStart + i;
        mesh.vertices[idx * 3] = r * cosf(theta);
        mesh.vertices[idx * 3 + 1] = length * 0.5f;
        mesh.vertices[idx * 3 + 2] = r * sinf(theta);
        mesh.texcoords[idx * 2] = u;
        mesh.texcoords[idx * 2 + 1] = 1.0f;
        mesh.colors[idx * 4] = 190; mesh.colors[idx * 4 + 1] = 160;
        mesh.colors[idx * 4 + 2] = 150; mesh.colors[idx * 4 + 3] = 255;

        r = radius * 0.85f;
        idx = bottomRingStart + i;
        mesh.vertices[idx * 3] = r * cosf(theta);
        mesh.vertices[idx * 3 + 1] = -length * 0.5f;
        mesh.vertices[idx * 3 + 2] = r * sinf(theta);
        mesh.texcoords[idx * 2] = u;
        mesh.texcoords[idx * 2 + 1] = 0.0f;
        mesh.colors[idx * 4] = 190; mesh.colors[idx * 4 + 1] = 160;
        mesh.colors[idx * 4 + 2] = 150; mesh.colors[idx * 4 + 3] = 255;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = topCenter;
        mesh.indices[ii++] = topRingStart + ((i + 1) % radialSegs);
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = bottomCenter;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + ((i + 1) % radialSegs);
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateHand(float scale) {
    int palmSegs = 16;
    int palmHSegs = 8;
    int palmVerts = (palmSegs + 1) * (palmHSegs + 1);
    int palmTris = palmSegs * palmHSegs * 2;

    int fingerCount = 4;
    int fingerSegs = 6;
    int fingerHSegs = 4;
    int fingerVerts = (fingerSegs + 1) * (fingerHSegs + 1);
    int fingerTris = fingerSegs * fingerHSegs * 2;

    int thumbSegs = 5;
    int thumbHSegs = 4;
    int thumbVerts = (thumbSegs + 1) * (thumbHSegs + 1);
    int thumbTris = thumbSegs * thumbHSegs * 2;

    int vertexCount = palmVerts + fingerCount * fingerVerts + thumbVerts;
    int triangleCount = palmTris + fingerCount * fingerTris + thumbTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    int vi = 0;
    float palmW = 0.08f * scale;
    float palmH = 0.12f * scale;
    float palmD = 0.04f * scale;

    for (int j = 0; j <= palmHSegs; j++) {
        float v = (float)j / palmHSegs;
        float y = v * palmH - palmH * 0.5f;
        for (int i = 0; i <= palmSegs; i++) {
            float u = (float)i / palmSegs;
            float theta = u * 2.0f * PI;
            float rx = palmW * (1.0f - 0.15f * v);
            float rz = palmD * (1.0f + 0.1f * sinf(v * PI));
            float x = rx * cosf(theta);
            float z = rz * sinf(theta);
            mesh.vertices[vi * 3] = x;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = z;
            mesh.texcoords[vi * 2] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4] = 190;
            mesh.colors[vi * 4 + 1] = 160;
            mesh.colors[vi * 4 + 2] = 150;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int fingerBase = vi;
    for (int f = 0; f < fingerCount; f++) {
        float fx = (f - (fingerCount - 1) * 0.5f) * palmW * 0.5f;
        float fy = palmH * 0.5f;
        float fz = palmD * 0.5f;
        for (int j = 0; j <= fingerHSegs; j++) {
            float v = (float)j / fingerHSegs;
            float y = fy + v * palmH * 0.8f;
            float r = 0.012f * scale * (1.0f - 0.3f * v);
            for (int i = 0; i <= fingerSegs; i++) {
                float u = (float)i / fingerSegs;
                float theta = u * 2.0f * PI;
                float x = fx + r * cosf(theta);
                float z = fz + r * sinf(theta);
                mesh.vertices[vi * 3] = x;
                mesh.vertices[vi * 3 + 1] = y;
                mesh.vertices[vi * 3 + 2] = z;
                mesh.texcoords[vi * 2] = u;
                mesh.texcoords[vi * 2 + 1] = v;
                mesh.colors[vi * 4] = 190;
                mesh.colors[vi * 4 + 1] = 160;
                mesh.colors[vi * 4 + 2] = 150;
                mesh.colors[vi * 4 + 3] = 255;
                vi++;
            }
        }
    }

    int thumbBase = vi;
    {
        float tx = -palmW * 0.7f;
        float ty = 0;
        float tz = palmD * 0.3f;
        for (int j = 0; j <= thumbHSegs; j++) {
            float v = (float)j / thumbHSegs;
            float y = ty + v * palmH * 0.6f;
            float r = 0.014f * scale * (1.0f - 0.25f * v);
            for (int i = 0; i <= thumbSegs; i++) {
                float u = (float)i / thumbSegs;
                float theta = u * 2.0f * PI;
                float x = tx + r * cosf(theta);
                float z = tz + r * sinf(theta);
                mesh.vertices[vi * 3] = x;
                mesh.vertices[vi * 3 + 1] = y;
                mesh.vertices[vi * 3 + 2] = z;
                mesh.texcoords[vi * 2] = u;
                mesh.texcoords[vi * 2 + 1] = v;
                mesh.colors[vi * 4] = 190;
                mesh.colors[vi * 4 + 1] = 160;
                mesh.colors[vi * 4 + 2] = 150;
                mesh.colors[vi * 4 + 3] = 255;
                vi++;
            }
        }
    }

    int ii = 0;
    for (int j = 0; j < palmHSegs; j++) {
        for (int i = 0; i < palmSegs; i++) {
            int a = j * (palmSegs + 1) + i;
            int b = a + 1;
            int c = (j + 1) * (palmSegs + 1) + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }

    for (int f = 0; f < fingerCount; f++) {
        int fBase = fingerBase + f * fingerVerts;
        for (int j = 0; j < fingerHSegs; j++) {
            for (int i = 0; i < fingerSegs; i++) {
                int a = fBase + j * (fingerSegs + 1) + i;
                int b = a + 1;
                int c = fBase + (j + 1) * (fingerSegs + 1) + i;
                int d = c + 1;
                mesh.indices[ii++] = a;
                mesh.indices[ii++] = c;
                mesh.indices[ii++] = b;
                mesh.indices[ii++] = b;
                mesh.indices[ii++] = c;
                mesh.indices[ii++] = d;
            }
        }
    }

    for (int j = 0; j < thumbHSegs; j++) {
        for (int i = 0; i < thumbSegs; i++) {
            int a = thumbBase + j * (thumbSegs + 1) + i;
            int b = a + 1;
            int c = thumbBase + (j + 1) * (thumbSegs + 1) + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateFoot(float scale) {
    int segs = 20;
    int hSegs = 10;
    int bodyVerts = (segs + 1) * (hSegs + 1);
    int quadCount = segs * hSegs;
    int triangleCount = quadCount * 2;

    int toeCount = 5;
    int toeSegs = 8;
    int toeHSegs = 4;
    int toeVerts = (toeSegs + 1) * (toeHSegs + 1);
    int toeTris = toeSegs * toeHSegs * 2;

    int vertexCount = bodyVerts + toeCount * toeVerts;
    int totalTris = triangleCount + toeCount * toeTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = totalTris;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(totalTris * 3, sizeof(unsigned short));

    int vi = 0;
    float footLen = 0.22f * scale;
    float footH = 0.08f * scale;
    float footD = 0.12f * scale;

    for (int j = 0; j <= hSegs; j++) {
        float v = (float)j / hSegs;
        float x = v * footLen - footLen * 0.2f;
        float arch = 0.015f * scale * sinf(v * PI);
        for (int i = 0; i <= segs; i++) {
            float u = (float)i / segs;
            float theta = u * 2.0f * PI;
            float halfTheta = theta * 0.5f;
            float rz = footD * 0.5f * (1.0f - 0.3f * fabsf(sinf(halfTheta)));
            float ry = footH * 0.5f * (1.0f + 0.2f * sinf(v * PI * 2.0f));
            float y = arch + rz * sinf(theta);
            float z = ry * cosf(theta);
            mesh.vertices[vi * 3] = x;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = z;
            mesh.texcoords[vi * 2] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4] = 190;
            mesh.colors[vi * 4 + 1] = 160;
            mesh.colors[vi * 4 + 2] = 150;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int toeBase = vi;
    for (int t = 0; t < toeCount; t++) {
        float tx = footLen * 0.6f + (t - (toeCount - 1) * 0.5f) * footD * 0.3f;
        float ty = footH * 0.3f;
        float tz = 0;
        for (int j = 0; j <= toeHSegs; j++) {
            float v = (float)j / toeHSegs;
            float y = ty + v * footH * 0.6f;
            float r = 0.015f * scale * (1.0f - 0.2f * v);
            for (int i = 0; i <= toeSegs; i++) {
                float u = (float)i / toeSegs;
                float theta = u * 2.0f * PI;
                float x = tx + r * cosf(theta);
                float z = tz + r * sinf(theta);
                mesh.vertices[vi * 3] = x;
                mesh.vertices[vi * 3 + 1] = y;
                mesh.vertices[vi * 3 + 2] = z;
                mesh.texcoords[vi * 2] = u;
                mesh.texcoords[vi * 2 + 1] = v;
                mesh.colors[vi * 4] = 190;
                mesh.colors[vi * 4 + 1] = 160;
                mesh.colors[vi * 4 + 2] = 150;
                mesh.colors[vi * 4 + 3] = 255;
                vi++;
            }
        }
    }

    int ii = 0;
    for (int j = 0; j < hSegs; j++) {
        for (int i = 0; i < segs; i++) {
            int a = j * (segs + 1) + i;
            int b = a + 1;
            int c = (j + 1) * (segs + 1) + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }

    for (int t = 0; t < toeCount; t++) {
        int tBase = toeBase + t * toeVerts;
        for (int j = 0; j < toeHSegs; j++) {
            for (int i = 0; i < toeSegs; i++) {
                int a = tBase + j * (toeSegs + 1) + i;
                int b = a + 1;
                int c = tBase + (j + 1) * (toeSegs + 1) + i;
                int d = c + 1;
                mesh.indices[ii++] = a;
                mesh.indices[ii++] = c;
                mesh.indices[ii++] = b;
                mesh.indices[ii++] = b;
                mesh.indices[ii++] = c;
                mesh.indices[ii++] = d;
            }
        }
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateSpine(float height) {
    int boneCount = 24;
    float segmentH = height / (float)boneCount;
    float radius = 0.06f;

    int radialSegs = 24;
    int heightSegs = 24;
    int ringVerts = radialSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = radialSegs * heightSegs;
    int triangleCount = quadCount * 2;
    int topCenter = bodyVerts;
    int bottomCenter = bodyVerts + 1;
    int topRingStart = bottomCenter + 1;
    int bottomRingStart = topRingStart + radialSegs;
    int vertexCount = bottomRingStart + radialSegs;
    int capTris = radialSegs * 2;
    triangleCount += capTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float ny = v * 2.0f - 1.0f;
        float y = ny * height * 0.5f;
        float r = radius;
        float vertebraPhase = ny * boneCount * 0.5f * PI;
        r += 0.03f * sinf(vertebraPhase);
        r += 0.015f * sinf(vertebraPhase * 2.3f);
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float x = r * cosf(theta);
            float z = r * sinf(theta);
            int idx = j * ringVerts + i;
            mesh.vertices[idx * 3] = x;
            mesh.vertices[idx * 3 + 1] = y;
            mesh.vertices[idx * 3 + 2] = z;
            mesh.texcoords[idx * 2] = u;
            mesh.texcoords[idx * 2 + 1] = v;
            mesh.colors[idx * 4] = 235;
            mesh.colors[idx * 4 + 1] = 225;
            mesh.colors[idx * 4 + 2] = 210;
            mesh.colors[idx * 4 + 3] = 255;
        }
    }

    mesh.vertices[topCenter * 3] = 0;
    mesh.vertices[topCenter * 3 + 1] = height * 0.5f;
    mesh.vertices[topCenter * 3 + 2] = 0;
    mesh.texcoords[topCenter * 2] = 0.5f;
    mesh.texcoords[topCenter * 2 + 1] = 1.0f;
    mesh.colors[topCenter * 4] = 235; mesh.colors[topCenter * 4 + 1] = 225;
    mesh.colors[topCenter * 4 + 2] = 210; mesh.colors[topCenter * 4 + 3] = 255;

    mesh.vertices[bottomCenter * 3] = 0;
    mesh.vertices[bottomCenter * 3 + 1] = -height * 0.5f;
    mesh.vertices[bottomCenter * 3 + 2] = 0;
    mesh.texcoords[bottomCenter * 2] = 0.5f;
    mesh.texcoords[bottomCenter * 2 + 1] = 0.0f;
    mesh.colors[bottomCenter * 4] = 235; mesh.colors[bottomCenter * 4 + 1] = 225;
    mesh.colors[bottomCenter * 4 + 2] = 210; mesh.colors[bottomCenter * 4 + 3] = 255;

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = topCenter;
        mesh.indices[ii++] = topRingStart + ((i + 1) % radialSegs);
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = bottomCenter;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + ((i + 1) % radialSegs);
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreateRibcage(float width, float height) {
    int ribCount = 12;
    float ribW = width * 0.8f;
    float ribH = height * 0.8f;
    float ribDepth = 0.02f;

    int radialSegs = 32;
    int heightSegs = 32;
    int ringVerts = radialSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = radialSegs * heightSegs;
    int triangleCount = quadCount * 2;
    int topCenter = bodyVerts;
    int bottomCenter = bodyVerts + 1;
    int topRingStart = bottomCenter + 1;
    int bottomRingStart = topRingStart + radialSegs;
    int vertexCount = bottomRingStart + radialSegs;
    int capTris = radialSegs * 2;
    triangleCount += capTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float ny = v * 2.0f - 1.0f;
        float y = ny * ribH * 0.5f;
        float ribPhase = ny * ribCount * 0.5f * PI;
        float r = ribW * 0.5f + 0.04f * sinf(ribPhase);
        r += 0.02f * sinf(ribPhase * 2.1f);
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float x = r * cosf(theta);
            float z = ribDepth * sinf(theta);
            int idx = j * ringVerts + i;
            mesh.vertices[idx * 3] = x;
            mesh.vertices[idx * 3 + 1] = y;
            mesh.vertices[idx * 3 + 2] = z;
            mesh.texcoords[idx * 2] = u;
            mesh.texcoords[idx * 2 + 1] = v;
            mesh.colors[idx * 4] = 230;
            mesh.colors[idx * 4 + 1] = 225;
            mesh.colors[idx * 4 + 2] = 210;
            mesh.colors[idx * 4 + 3] = 255;
        }
    }

    mesh.vertices[topCenter * 3] = 0;
    mesh.vertices[topCenter * 3 + 1] = ribH * 0.5f;
    mesh.vertices[topCenter * 3 + 2] = 0;
    mesh.texcoords[topCenter * 2] = 0.5f;
    mesh.texcoords[topCenter * 2 + 1] = 1.0f;
    mesh.colors[topCenter * 4] = 230; mesh.colors[topCenter * 4 + 1] = 225;
    mesh.colors[topCenter * 4 + 2] = 210; mesh.colors[topCenter * 4 + 3] = 255;

    mesh.vertices[bottomCenter * 3] = 0;
    mesh.vertices[bottomCenter * 3 + 1] = -ribH * 0.5f;
    mesh.vertices[bottomCenter * 3 + 2] = 0;
    mesh.texcoords[bottomCenter * 2] = 0.5f;
    mesh.texcoords[bottomCenter * 2 + 1] = 0.0f;
    mesh.colors[bottomCenter * 4] = 230; mesh.colors[bottomCenter * 4 + 1] = 225;
    mesh.colors[bottomCenter * 4 + 2] = 210; mesh.colors[bottomCenter * 4 + 3] = 255;

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = topCenter;
        mesh.indices[ii++] = topRingStart + ((i + 1) % radialSegs);
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = bottomCenter;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + ((i + 1) % radialSegs);
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

Mesh ZombieMesh_CreatePelvis(float width, float height) {
    int radialSegs = 32;
    int heightSegs = 16;
    int ringVerts = radialSegs + 1;
    int bodyVerts = ringVerts * (heightSegs + 1);
    int quadCount = radialSegs * heightSegs;
    int triangleCount = quadCount * 2;
    int topCenter = bodyVerts;
    int bottomCenter = bodyVerts + 1;
    int topRingStart = bottomCenter + 1;
    int bottomRingStart = topRingStart + radialSegs;
    int vertexCount = bottomRingStart + radialSegs;
    int capTris = radialSegs * 2;
    triangleCount += capTris;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
    mesh.normals = (float *)calloc(vertexCount * 3, sizeof(float));
    mesh.colors = (unsigned char *)calloc(vertexCount * 4, sizeof(unsigned char));
    mesh.indices = (unsigned short *)calloc(triangleCount * 3, sizeof(unsigned short));

    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float y = v * height - height * 0.5f;
        float t = v;
        float r = width * 0.5f * (1.0f - 0.2f * t);
        r *= (1.0f + 0.1f * sinf(t * PI));
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float x = r * cosf(theta);
            float z = r * sinf(theta) * 0.6f;
            int idx = j * ringVerts + i;
            mesh.vertices[idx * 3] = x;
            mesh.vertices[idx * 3 + 1] = y;
            mesh.vertices[idx * 3 + 2] = z;
            mesh.texcoords[idx * 2] = u;
            mesh.texcoords[idx * 2 + 1] = v;
            mesh.colors[idx * 4] = 235;
            mesh.colors[idx * 4 + 1] = 225;
            mesh.colors[idx * 4 + 2] = 210;
            mesh.colors[idx * 4 + 3] = 255;
        }
    }

    mesh.vertices[topCenter * 3] = 0;
    mesh.vertices[topCenter * 3 + 1] = height * 0.5f;
    mesh.vertices[topCenter * 3 + 2] = 0;
    mesh.texcoords[topCenter * 2] = 0.5f;
    mesh.texcoords[topCenter * 2 + 1] = 1.0f;
    mesh.colors[topCenter * 4] = 235; mesh.colors[topCenter * 4 + 1] = 225;
    mesh.colors[topCenter * 4 + 2] = 210; mesh.colors[topCenter * 4 + 3] = 255;

    mesh.vertices[bottomCenter * 3] = 0;
    mesh.vertices[bottomCenter * 3 + 1] = -height * 0.5f;
    mesh.vertices[bottomCenter * 3 + 2] = 0;
    mesh.texcoords[bottomCenter * 2] = 0.5f;
    mesh.texcoords[bottomCenter * 2 + 1] = 0.0f;
    mesh.colors[bottomCenter * 4] = 235; mesh.colors[bottomCenter * 4 + 1] = 225;
    mesh.colors[bottomCenter * 4 + 2] = 210; mesh.colors[bottomCenter * 4 + 3] = 255;

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + 1;
            int c = (j + 1) * ringVerts + i;
            int d = c + 1;
            mesh.indices[ii++] = a;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = b;
            mesh.indices[ii++] = c;
            mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = topCenter;
        mesh.indices[ii++] = topRingStart + ((i + 1) % radialSegs);
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        mesh.indices[ii++] = bottomCenter;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + ((i + 1) % radialSegs);
    }

    ComputeMeshNormals(&mesh);
    return mesh;
}

