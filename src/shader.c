#include "shader.h"
#include "raylib.h"

static const char *pbrVertShader = 
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec3 vertexNormal;\n"
    "in vec2 vertexTexCoord;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n"
    "uniform mat4 normalMatrix;\n"
    "out vec3 fragPosition;\n"
    "out vec3 fragNormal;\n"
    "out vec2 fragTexCoord;\n"
    "void main() {\n"
    "    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));\n"
    "    fragNormal = mat3(normalMatrix) * vertexNormal;\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *pbrFragShader = 
    "#version 330\n"
    "in vec3 fragPosition;\n"
    "in vec3 fragNormal;\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D albedoMap;\n"
    "uniform vec3 lightPos[4];\n"
    "uniform vec3 lightCol[4];\n"
    "uniform int lightCount;\n"
    "uniform float metallic;\n"
    "uniform float roughness;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec3 albedo = texture(albedoMap, fragTexCoord).rgb;\n"
    "    vec3 N = normalize(fragNormal);\n"
    "    vec3 V = normalize(-fragPosition);\n"
    "    vec3 Lo = vec3(0.0);\n"
    "    for (int i = 0; i < lightCount; i++) {\n"
    "        vec3 L = normalize(lightPos[i] - fragPosition);\n"
    "        vec3 H = normalize(V + L);\n"
    "        float dist = length(lightPos[i] - fragPosition);\n"
    "        float attenuation = 1.0 / (1.0 + 0.05 * dist + 0.01 * dist * dist);\n"
    "        float diff = max(dot(N, L), 0.0);\n"
    "        float spec = pow(max(dot(N, H), 0.0), 16.0 * (1.0 - roughness));\n"
    "        vec3 radiance = lightCol[i] * attenuation * 1.5;\n"
    "        Lo += (albedo * diff + vec3(1.0) * spec * (1.0 - metallic)) * radiance;\n"
    "    }\n"
    "    vec3 ambient = albedo * 0.4;\n"
    "    finalColor = vec4(ambient + Lo, 1.0);\n"
    "}\n";

static const char *postVertShader = 
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "uniform mat4 mvp;\n"
    "out vec2 fragTexCoord;\n"
    "void main() {\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *postFragShader = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "uniform vec2 resolution;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec3 col = texture(texture0, uv).rgb;\n"
    "    float vignette = 1.0 - smoothstep(0.3, 1.2, length(uv - 0.5) * 1.6);\n"
    "    col *= mix(0.6, 1.0, vignette);\n"
    "    float grain = (fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 0.08;\n"
    "    col += grain;\n"
    "    float bloom = smoothstep(0.6, 1.0, max(col.r, max(col.g, col.b))) * 0.4;\n"
    "    col += bloom;\n"
    "    float fog = smoothstep(0.3, 1.0, length(uv - 0.5) * 1.3);\n"
    "    col = mix(col, vec3(0.3, 0.2, 0.4), fog * 0.5);\n"
    "    col = pow(col, vec3(1.0 / 2.2));\n"
    "    finalColor = vec4(col, 1.0);\n"
    "}\n";

static const char *scopeVertShader = 
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "uniform mat4 mvp;\n"
    "out vec2 fragTexCoord;\n"
    "void main() {\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *scopeFragShader = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "uniform vec2 resolution;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec2 uv = fragTexCoord - 0.5;\n"
    "    float dist = length(uv);\n"
    "    float ring = smoothstep(0.48, 0.5, dist) - smoothstep(0.5, 0.52, dist);\n"
    "    float crosshair = 0.0;\n"
    "    if (abs(uv.x) < 0.002 || abs(uv.y) < 0.002) crosshair = 1.0;\n"
    "    if (dist > 0.5) discard;\n"
    "    vec3 col = texture(texture0, fragTexCoord).rgb;\n"
    "    col = mix(col, vec3(0.0), ring * 0.8);\n"
    "    col = mix(col, vec3(0.8, 0.2, 0.2), crosshair * 0.9);\n"
    "    finalColor = vec4(col, 1.0);\n"
    "}\n";

void ShaderInit(ShaderManager *shaders, int screenWidth, int screenHeight) {
    shaders->pbr = LoadShaderFromMemory(pbrVertShader, pbrFragShader);
    shaders->postProcess = LoadShaderFromMemory(postVertShader, postFragShader);
    shaders->scope = LoadShaderFromMemory(scopeVertShader, scopeFragShader);
    shaders->pbrLocAlbedo = GetShaderLocation(shaders->pbr, "albedoMap");
    shaders->pbrLocLightPos[0] = GetShaderLocation(shaders->pbr, "lightPos[0]");
    shaders->pbrLocLightCol[0] = GetShaderLocation(shaders->pbr, "lightCol[0]");
    shaders->pbrLocLightCount = GetShaderLocation(shaders->pbr, "lightCount");
    shaders->pbrLocMetallic = GetShaderLocation(shaders->pbr, "metallic");
    shaders->pbrLocRoughness = GetShaderLocation(shaders->pbr, "roughness");
    shaders->postLocTime = GetShaderLocation(shaders->postProcess, "time");
    shaders->postLocResolution = GetShaderLocation(shaders->postProcess, "resolution");
    shaders->scopeLocTime = GetShaderLocation(shaders->scope, "time");
    shaders->scopeLocResolution = GetShaderLocation(shaders->scope, "resolution");
}

void ShaderUpdate(ShaderManager *shaders, float time, int screenWidth, int screenHeight) {
    float t = (float)time;
    SetShaderValue(shaders->postProcess, shaders->postLocTime, &t, SHADER_UNIFORM_FLOAT);
    Vector2 res = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(shaders->postProcess, shaders->postLocResolution, &res, SHADER_UNIFORM_VEC2);
    SetShaderValue(shaders->scope, shaders->scopeLocTime, &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaders->scope, shaders->scopeLocResolution, &res, SHADER_UNIFORM_VEC2);
}

void ShaderBeginPBR(ShaderManager *shaders) {
    BeginShaderMode(shaders->pbr);
}

void ShaderBeginPostProcess(ShaderManager *shaders) {
    BeginShaderMode(shaders->postProcess);
}

void ShaderBeginScope(ShaderManager *shaders) {
    BeginShaderMode(shaders->scope);
}

void ShaderEnd(ShaderManager *shaders) {
    EndShaderMode();
    (void)shaders;
}

void ShaderShutdown(ShaderManager *shaders) {
    UnloadShader(shaders->pbr);
    UnloadShader(shaders->postProcess);
    UnloadShader(shaders->scope);
}
