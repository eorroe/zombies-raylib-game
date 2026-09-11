#include "player_mesh.h"
#include <stdlib.h>
#include <math.h>
#include "raymath.h"

static float TorsoRadius(float ny, float theta, float width, float height, float depth) {
    float baseR = 0.5f * (width + depth) * 0.5f;
    float taper = 1.0f - 0.3f * ny * ny;
    float shoulder = 1.0f + 0.25f * powf(fmaxf(0.0f, ny - 0.3f), 2.0f);
    float waist = 1.0f - 0.15f * powf(fmaxf(0.0f, -ny), 3.0f);
    float angleMod = 1.0f;
    angleMod += 0.03f * sinf(theta * 5.0f + ny * 3.0f);
    angleMod += 0.02f * sinf(theta * 8.0f - ny * 2.0f);
    return baseR * taper * shoulder * waist * angleMod;
}

Mesh PlayerMesh_CreateTorso(float width, float height, float depth) {
    int radialSegs = 64;
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

            mesh.vertices[vi * 3 + 0] = ct * r;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = st * r;
            mesh.texcoords[vi * 2 + 0] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4 + 0] = 200;
            mesh.colors[vi * 4 + 1] = 180;
            mesh.colors[vi * 4 + 2] = 170;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int topCenterIdx = vi++;
    int bottomCenterIdx = vi++;
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        float r = TorsoRadius(1.0f, theta, width, height, depth);
        mesh.vertices[vi * 3 + 0] = cosf(theta) * r;
        mesh.vertices[vi * 3 + 1] = height * 0.5f;
        mesh.vertices[vi * 3 + 2] = sinf(theta) * r;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 1.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 170;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        float r = TorsoRadius(-1.0f, theta, width, height, depth);
        mesh.vertices[vi * 3 + 0] = cosf(theta) * r;
        mesh.vertices[vi * 3 + 1] = -height * 0.5f;
        mesh.vertices[vi * 3 + 2] = sinf(theta) * r;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 0.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 170;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + ringVerts;
            int c = a + 1;
            int d = b + 1;
            mesh.indices[ii++] = a; mesh.indices[ii++] = c; mesh.indices[ii++] = b;
            mesh.indices[ii++] = b; mesh.indices[ii++] = c; mesh.indices[ii++] = d;
        }
    }

    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = topCenterIdx;
        mesh.indices[ii++] = topRingStart + next;
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = bottomCenterIdx;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + next;
    }

    PlayerMesh_ComputeNormals(&mesh);
    return mesh;
}

Mesh PlayerMesh_CreateHead(float radius) {
    int radialSegs = 48;
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

    int vi = 0;
    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float phi = v * PI;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float ct = cosf(theta);
            float st = sinf(theta);
            float sp = sinf(phi);
            float cp = cosf(phi);

            float x = radius * sp * ct;
            float y = radius * cp;
            float z = radius * sp * st;

            float brow = 0.0f;
            if (phi > 1.2f && phi < 2.0f && fabsf(st) < 0.4f) {
                brow = -0.015f * sinf(phi * 3.0f) * (1.0f - fabsf(st) / 0.4f);
            }
            y += brow;

            mesh.vertices[vi * 3 + 0] = x;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = z;
            mesh.texcoords[vi * 2 + 0] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4 + 0] = 200;
            mesh.colors[vi * 4 + 1] = 180;
            mesh.colors[vi * 4 + 2] = 160;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int topCenterIdx = vi++;
    int bottomCenterIdx = vi++;
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = radius;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 1.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 160;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = -radius * 0.5f;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 0.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 160;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + ringVerts;
            int c = a + 1;
            int d = b + 1;
            mesh.indices[ii++] = a; mesh.indices[ii++] = c; mesh.indices[ii++] = b;
            mesh.indices[ii++] = b; mesh.indices[ii++] = c; mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = topCenterIdx;
        mesh.indices[ii++] = topRingStart + next;
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = bottomCenterIdx;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + next;
    }

    PlayerMesh_ComputeNormals(&mesh);
    return mesh;
}

Mesh PlayerMesh_CreateHelmet(float radius) {
    int radialSegs = 48;
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

    int vi = 0;
    for (int j = 0; j <= heightSegs; j++) {
        float v = (float)j / heightSegs;
        float phi = v * PI * 0.5f;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float sp = sinf(phi);
            float cp = cosf(phi);

            float x = radius * sp * cosf(theta);
            float y = radius * cp;
            float z = radius * sp * sinf(theta);

            float brim = 0.0f;
            if (phi < 0.4f) {
                brim = 0.02f * sinf(phi / 0.4f * PI);
            }

            mesh.vertices[vi * 3 + 0] = x;
            mesh.vertices[vi * 3 + 1] = y + brim;
            mesh.vertices[vi * 3 + 2] = z;
            mesh.texcoords[vi * 2 + 0] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4 + 0] = 40;
            mesh.colors[vi * 4 + 1] = 50;
            mesh.colors[vi * 4 + 2] = 70;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int topCenterIdx = vi++;
    int bottomCenterIdx = vi++;
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = radius;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 1.0f;
        mesh.colors[vi * 4 + 0] = 40;
        mesh.colors[vi * 4 + 1] = 50;
        mesh.colors[vi * 4 + 2] = 70;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = 0;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 0.0f;
        mesh.colors[vi * 4 + 0] = 40;
        mesh.colors[vi * 4 + 1] = 50;
        mesh.colors[vi * 4 + 2] = 70;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + ringVerts;
            int c = a + 1;
            int d = b + 1;
            mesh.indices[ii++] = a; mesh.indices[ii++] = c; mesh.indices[ii++] = b;
            mesh.indices[ii++] = b; mesh.indices[ii++] = c; mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = topCenterIdx;
        mesh.indices[ii++] = topRingStart + next;
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = bottomCenterIdx;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + next;
    }

    PlayerMesh_ComputeNormals(&mesh);
    return mesh;
}

Mesh PlayerMesh_CreateLimb(float radius, float length, int slices) {
    int radialSegs = slices;
    int heightSegs = 8;
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
        float y = ny * length * 0.5f;
        float taper = 1.0f - 0.2f * ny * ny;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float r = radius * taper;

            mesh.vertices[vi * 3 + 0] = cosf(theta) * r;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = sinf(theta) * r;
            mesh.texcoords[vi * 2 + 0] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4 + 0] = 200;
            mesh.colors[vi * 4 + 1] = 180;
            mesh.colors[vi * 4 + 2] = 170;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int topCenterIdx = vi++;
    int bottomCenterIdx = vi++;
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = length * 0.5f;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 1.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 170;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = -length * 0.5f;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 0.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 170;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + ringVerts;
            int c = a + 1;
            int d = b + 1;
            mesh.indices[ii++] = a; mesh.indices[ii++] = c; mesh.indices[ii++] = b;
            mesh.indices[ii++] = b; mesh.indices[ii++] = c; mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = topCenterIdx;
        mesh.indices[ii++] = topRingStart + next;
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = bottomCenterIdx;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + next;
    }

    PlayerMesh_ComputeNormals(&mesh);
    return mesh;
}

Mesh PlayerMesh_CreateHand(float radius) {
    int radialSegs = 16;
    int heightSegs = 8;
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
        float y = ny * radius * 2.0f * 0.5f;
        float taper = 1.0f - 0.3f * ny * ny;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float r = radius * taper;

            mesh.vertices[vi * 3 + 0] = cosf(theta) * r;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = sinf(theta) * r;
            mesh.texcoords[vi * 2 + 0] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4 + 0] = 200;
            mesh.colors[vi * 4 + 1] = 180;
            mesh.colors[vi * 4 + 2] = 170;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int topCenterIdx = vi++;
    int bottomCenterIdx = vi++;
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = radius;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 1.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 170;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = -radius;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 0.0f;
        mesh.colors[vi * 4 + 0] = 200;
        mesh.colors[vi * 4 + 1] = 180;
        mesh.colors[vi * 4 + 2] = 170;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + ringVerts;
            int c = a + 1;
            int d = b + 1;
            mesh.indices[ii++] = a; mesh.indices[ii++] = c; mesh.indices[ii++] = b;
            mesh.indices[ii++] = b; mesh.indices[ii++] = c; mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = topCenterIdx;
        mesh.indices[ii++] = topRingStart + next;
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = bottomCenterIdx;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + next;
    }

    PlayerMesh_ComputeNormals(&mesh);
    return mesh;
}

Mesh PlayerMesh_CreateFoot(float radius) {
    int radialSegs = 16;
    int heightSegs = 8;
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
        float y = ny * radius * 1.5f * 0.5f;
        float taper = 1.0f - 0.1f * ny * ny;
        for (int i = 0; i <= radialSegs; i++) {
            float u = (float)i / radialSegs;
            float theta = u * 2.0f * PI;
            float r = radius * taper;

            mesh.vertices[vi * 3 + 0] = cosf(theta) * r;
            mesh.vertices[vi * 3 + 1] = y;
            mesh.vertices[vi * 3 + 2] = sinf(theta) * r;
            mesh.texcoords[vi * 2 + 0] = u;
            mesh.texcoords[vi * 2 + 1] = v;
            mesh.colors[vi * 4 + 0] = 60;
            mesh.colors[vi * 4 + 1] = 100;
            mesh.colors[vi * 4 + 2] = 160;
            mesh.colors[vi * 4 + 3] = 255;
            vi++;
        }
    }

    int topCenterIdx = vi++;
    int bottomCenterIdx = vi++;
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = radius * 0.75f;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 1.0f;
        mesh.colors[vi * 4 + 0] = 60;
        mesh.colors[vi * 4 + 1] = 100;
        mesh.colors[vi * 4 + 2] = 160;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }
    for (int i = 0; i < radialSegs; i++) {
        float theta = (float)i / radialSegs * 2.0f * PI;
        mesh.vertices[vi * 3 + 0] = 0;
        mesh.vertices[vi * 3 + 1] = -radius * 0.75f;
        mesh.vertices[vi * 3 + 2] = 0;
        mesh.texcoords[vi * 2 + 0] = (float)i / radialSegs;
        mesh.texcoords[vi * 2 + 1] = 0.0f;
        mesh.colors[vi * 4 + 0] = 60;
        mesh.colors[vi * 4 + 1] = 100;
        mesh.colors[vi * 4 + 2] = 160;
        mesh.colors[vi * 4 + 3] = 255;
        vi++;
    }

    int ii = 0;
    for (int j = 0; j < heightSegs; j++) {
        for (int i = 0; i < radialSegs; i++) {
            int a = j * ringVerts + i;
            int b = a + ringVerts;
            int c = a + 1;
            int d = b + 1;
            mesh.indices[ii++] = a; mesh.indices[ii++] = c; mesh.indices[ii++] = b;
            mesh.indices[ii++] = b; mesh.indices[ii++] = c; mesh.indices[ii++] = d;
        }
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = topCenterIdx;
        mesh.indices[ii++] = topRingStart + next;
        mesh.indices[ii++] = topRingStart + i;
    }
    for (int i = 0; i < radialSegs; i++) {
        int next = (i + 1) % radialSegs;
        mesh.indices[ii++] = bottomCenterIdx;
        mesh.indices[ii++] = bottomRingStart + i;
        mesh.indices[ii++] = bottomRingStart + next;
    }

    PlayerMesh_ComputeNormals(&mesh);
    return mesh;
}

void PlayerMesh_ComputeNormals(Mesh *mesh) {
    for (int i = 0; i < mesh->vertexCount * 3; i++) {
        mesh->normals[i] = 0.0f;
    }
    for (int i = 0; i < mesh->triangleCount; i++) {
        int i0 = mesh->indices[i * 3 + 0];
        int i1 = mesh->indices[i * 3 + 1];
        int i2 = mesh->indices[i * 3 + 2];
        Vector3 v0 = { mesh->vertices[i0 * 3], mesh->vertices[i0 * 3 + 1], mesh->vertices[i0 * 3 + 2] };
        Vector3 v1 = { mesh->vertices[i1 * 3], mesh->vertices[i1 * 3 + 1], mesh->vertices[i1 * 3 + 2] };
        Vector3 v2 = { mesh->vertices[i2 * 3], mesh->vertices[i2 * 3 + 1], mesh->vertices[i2 * 3 + 2] };
        Vector3 edge1 = Vector3Subtract(v1, v0);
        Vector3 edge2 = Vector3Subtract(v2, v0);
        Vector3 normal = Vector3CrossProduct(edge1, edge2);
        mesh->normals[i0 * 3] += normal.x;
        mesh->normals[i0 * 3 + 1] += normal.y;
        mesh->normals[i0 * 3 + 2] += normal.z;
        mesh->normals[i1 * 3] += normal.x;
        mesh->normals[i1 * 3 + 1] += normal.y;
        mesh->normals[i1 * 3 + 2] += normal.z;
        mesh->normals[i2 * 3] += normal.x;
        mesh->normals[i2 * 3 + 1] += normal.y;
        mesh->normals[i2 * 3 + 2] += normal.z;
    }
    for (int i = 0; i < mesh->vertexCount; i++) {
        Vector3 n = { mesh->normals[i * 3], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
        float len = Vector3Length(n);
        if (len > 0.00001f) {
            mesh->normals[i * 3] /= len;
            mesh->normals[i * 3 + 1] /= len;
            mesh->normals[i * 3 + 2] /= len;
        }
    }
}

void PlayerMesh_Upload(Mesh *mesh) {
    UploadMesh(mesh, false);
}