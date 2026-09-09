#ifndef SHADER_H
#define SHADER_H

#include "raylib.h"

typedef struct {
    Shader pbr;
    Shader postProcess;
    Shader scope;
    int pbrLocAlbedo;
    int pbrLocNormal;
    int pbrLocMetallic;
    int pbrLocRoughness;
    int pbrLocFogColor;
    int pbrLocFogDensity;
    int pbrLocLightPos[8];
    int pbrLocLightCol[8];
    int pbrLocLightCount;
    int postLocTime;
    int postLocResolution;
    int scopeLocTime;
    int scopeLocResolution;
} ShaderManager;

void ShaderInit(ShaderManager *shaders, int screenWidth, int screenHeight);
void ShaderUpdate(ShaderManager *shaders, float time, int screenWidth, int screenHeight);
void ShaderBeginPBR(ShaderManager *shaders);
void ShaderBeginPostProcess(ShaderManager *shaders);
void ShaderBeginScope(ShaderManager *shaders);
void ShaderEnd(ShaderManager *shaders);
void ShaderSetFog(ShaderManager *shaders, Vector3 fogColor, float fogDensity);
void ShaderShutdown(ShaderManager *shaders);

#endif
