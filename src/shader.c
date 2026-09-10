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
    "    fragNormal = vertexNormal;\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *pbrFragShader = 
    "#version 330\n"
    "precision highp float;\n"
    "in vec3 fragPosition;\n"
    "in vec3 fragNormal;\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform sampler2D texture1;\n"
    "uniform vec3 lightPos[8];\n"
    "uniform vec3 lightCol[8];\n"
    "uniform int lightCount;\n"
    "uniform vec3 dirLightDir;\n"
    "uniform vec3 dirLightCol;\n"
    "uniform float metallic;\n"
    "uniform float roughness;\n"
    "uniform vec3 fogColor;\n"
    "uniform float fogDensity;\n"
    "out vec4 finalColor;\n"

    "const float PI = 3.14159265359;\n"

    "float DistributionGGX(vec3 N, vec3 H, float roughness) {\n"
    "    float a = roughness * roughness;\n"
    "    float a2 = a * a;\n"
    "    float NdotH = max(dot(N, H), 0.0);\n"
    "    float NdotH2 = NdotH * NdotH;\n"
    "    float num = a2;\n"
    "    float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n"
    "    denom = PI * denom * denom;\n"
    "    return num / max(denom, 0.0001);\n"
    "}\n"

    "float GeometrySchlickGGX(float NdotV, float roughness) {\n"
    "    float r = roughness + 1.0;\n"
    "    float k = (r * r) / 8.0;\n"
    "    return NdotV / (NdotV * (1.0 - k) + k);\n"
    "}\n"

    "float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {\n"
    "    float NdotV = max(dot(N, V), 0.0);\n"
    "    float NdotL = max(dot(N, L), 0.0);\n"
    "    return GeometrySchlickGGX(NdotV, roughness) *\n"
    "           GeometrySchlickGGX(NdotL, roughness);\n"
    "}\n"

    "vec3 FresnelSchlick(float cosTheta, vec3 F0) {\n"
    "    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);\n"
    "}\n"

    "void main() {\n"
    "    vec3 albedo = texture(texture0, fragTexCoord).rgb;\n"

    "    vec3 N = normalize(fragNormal);\n"
    "    vec3 normalTex = texture(texture1, fragTexCoord).rgb * 2.0 - 1.0;\n"
    "    N = normalize(mix(N, normalize(N + normalTex), 0.5));\n"

    "    vec3 V = normalize(-fragPosition);\n"
    "    vec3 R = reflect(-V, N);\n"

    "    float ao = 0.5 + 0.5 * N.y;\n"

    "    vec3 F0 = mix(vec3(0.04), albedo, metallic);\n"

    "    vec3 Lo = vec3(0.0);\n"
    "    for (int i = 0; i < 8; i++) {\n"
    "        if (i >= lightCount) break;\n"
    "        vec3 L = normalize(lightPos[i] - fragPosition);\n"
    "        float diff = max(dot(N, L), 0.0);\n"
    "        float attenuation = 1.0 / (1.0 + 0.08 * length(lightPos[i] - fragPosition));\n"
    "        vec3 radiance = lightCol[i] * attenuation * 1.2;\n"
    "        Lo += albedo * diff * radiance;\n"
    "    }\n"
    "    vec3 ambient = albedo * 0.55;\n"
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
    "precision highp float;\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "uniform vec2 resolution;\n"
    "out vec4 finalColor;\n"

    "const float PI = 3.14159265359;\n"

    "vec3 ACESFilm(vec3 x) {\n"
    "    float a = 2.51;\n"
    "    float b = 0.03;\n"
    "    float c = 2.43;\n"
    "    float d = 0.59;\n"
    "    float e = 0.14;\n"
    "    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);\n"
    "}\n"

    "vec3 sampleBloom(sampler2D tex, vec2 uv, float threshold) {\n"
    "    vec3 bloom = vec3(0.0);\n"
    "    float total = 0.0;\n"
    "    for (int x = -4; x <= 4; x++) {\n"
    "        for (int y = -4; y <= 4; y++) {\n"
    "            float weight = 1.0 / (1.0 + float(abs(x) + abs(y)));\n"
    "            vec2 offset = vec2(float(x), float(y)) / resolution * 4.0;\n"
    "            vec3 s = texture(tex, uv + offset).rgb;\n"
    "            float bright = max(s.r, max(s.g, s.b));\n"
    "            float contrib = smoothstep(threshold, 1.0, bright);\n"
    "            bloom += s * contrib * weight;\n"
    "            total += weight;\n"
    "        }\n"
    "    }\n"
    "    return bloom / total;\n"
    "}\n"

    "void main() {\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec3 col = texture(texture0, uv).rgb;\n"
    "    float offset = 1.5 / resolution.x;\n"
    "    vec3 n = texture(texture0, uv + vec2(0.0, offset)).rgb;\n"
    "    vec3 s = texture(texture0, uv - vec2(0.0, offset)).rgb;\n"
    "    vec3 e = texture(texture0, uv + vec2(offset, 0.0)).rgb;\n"
    "    vec3 w = texture(texture0, uv - vec2(offset, 0.0)).rgb;\n"
    "    float edge = length(n - s) + length(e - w);\n"
    "    edge = smoothstep(0.08, 0.35, edge);\n"
    "    col = floor(col * 7.0 + 0.5) / 7.0;\n"
    "    float paper = (fract(sin(dot(uv * resolution, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 0.06;\n"
    "    col += paper;\n"
    "    col = mix(col, vec3(0.96, 0.94, 0.88), 0.12);\n"
    "    col = mix(col, vec3(0.15, 0.15, 0.18), edge * 0.65);\n"
    "    float vignette = 1.0 - smoothstep(0.2, 1.0, length(uv - 0.5) * 1.8);\n"
    "    col *= mix(0.6, 1.0, vignette);\n"
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
    "precision highp float;\n"
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
    "    col = mix(col, vec3(0.15, 0.15, 0.18), ring * 0.9);\n"
    "    col = mix(col, vec3(0.15, 0.15, 0.18), crosshair * 0.9);\n"
    "    finalColor = vec4(col, 1.0);\n"
    "}\n";

void ShaderInit(ShaderManager *shaders, int screenWidth, int screenHeight) {
    shaders->pbr = LoadShaderFromMemory(pbrVertShader, pbrFragShader);
    shaders->postProcess = LoadShaderFromMemory(postVertShader, postFragShader);
    shaders->scope = LoadShaderFromMemory(scopeVertShader, scopeFragShader);

    shaders->pbrLocAlbedo = GetShaderLocation(shaders->pbr, "texture0");
    shaders->pbrLocNormal = GetShaderLocation(shaders->pbr, "texture1");
    shaders->pbrLocLightPos[0] = GetShaderLocation(shaders->pbr, "lightPos[0]");
    shaders->pbrLocLightPos[1] = GetShaderLocation(shaders->pbr, "lightPos[1]");
    shaders->pbrLocLightPos[2] = GetShaderLocation(shaders->pbr, "lightPos[2]");
    shaders->pbrLocLightPos[3] = GetShaderLocation(shaders->pbr, "lightPos[3]");
    shaders->pbrLocLightPos[4] = GetShaderLocation(shaders->pbr, "lightPos[4]");
    shaders->pbrLocLightPos[5] = GetShaderLocation(shaders->pbr, "lightPos[5]");
    shaders->pbrLocLightPos[6] = GetShaderLocation(shaders->pbr, "lightPos[6]");
    shaders->pbrLocLightPos[7] = GetShaderLocation(shaders->pbr, "lightPos[7]");
    shaders->pbrLocLightCol[0] = GetShaderLocation(shaders->pbr, "lightCol[0]");
    shaders->pbrLocLightCol[1] = GetShaderLocation(shaders->pbr, "lightCol[1]");
    shaders->pbrLocLightCol[2] = GetShaderLocation(shaders->pbr, "lightCol[2]");
    shaders->pbrLocLightCol[3] = GetShaderLocation(shaders->pbr, "lightCol[3]");
    shaders->pbrLocLightCol[4] = GetShaderLocation(shaders->pbr, "lightCol[4]");
    shaders->pbrLocLightCol[5] = GetShaderLocation(shaders->pbr, "lightCol[5]");
    shaders->pbrLocLightCol[6] = GetShaderLocation(shaders->pbr, "lightCol[6]");
    shaders->pbrLocLightCol[7] = GetShaderLocation(shaders->pbr, "lightCol[7]");
    shaders->pbrLocLightCount = GetShaderLocation(shaders->pbr, "lightCount");
    shaders->pbrLocDirLightDir = GetShaderLocation(shaders->pbr, "dirLightDir");
    shaders->pbrLocDirLightCol = GetShaderLocation(shaders->pbr, "dirLightCol");
    shaders->pbrLocMetallic = GetShaderLocation(shaders->pbr, "metallic");
    shaders->pbrLocFogColor = GetShaderLocation(shaders->pbr, "fogColor");
    shaders->pbrLocFogDensity = GetShaderLocation(shaders->pbr, "fogDensity");
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

void ShaderSetFog(ShaderManager *shaders, Vector3 fogColor, float fogDensity) {
    SetShaderValue(shaders->pbr, shaders->pbrLocFogColor, &fogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shaders->pbr, shaders->pbrLocFogDensity, &fogDensity, SHADER_UNIFORM_FLOAT);
}

void ShaderSetDirectionalLight(ShaderManager *shaders, Vector3 dir, Vector3 col) {
    SetShaderValue(shaders->pbr, shaders->pbrLocDirLightDir, &dir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shaders->pbr, shaders->pbrLocDirLightCol, &col, SHADER_UNIFORM_VEC3);
}

void ShaderShutdown(ShaderManager *shaders) {
    UnloadShader(shaders->pbr);
    UnloadShader(shaders->postProcess);
    UnloadShader(shaders->scope);
}
