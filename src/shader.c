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
    "        vec3 H = normalize(V + L);\n"
    "        float dist = length(lightPos[i] - fragPosition);\n"
     "    float attenuation = 1.0 / (1.0 + 0.05 * dist + 0.01 * dist * dist);\n"
     "        vec3 radiance = lightCol[i] * attenuation;\n"

    "        float NDF = DistributionGGX(N, H, roughness);\n"
    "        float G   = GeometrySmith(N, V, L, roughness);\n"
    "        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);\n"

    "        vec3 kS = F;\n"
    "        vec3 kD = (1.0 - kS) * (1.0 - metallic);\n"

    "        vec3 numerator    = NDF * G * F;\n"
    "        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);\n"
    "        vec3 specular     = numerator / max(denominator, 0.001);\n"

    "        float NdotL = max(dot(N, L), 0.0);\n"
    "        Lo += (kD * albedo / PI + specular) * radiance * NdotL;\n"
     "    }\n"
     "    vec3 Ldir = normalize(dirLightDir);\n"
    "    float NdotLdir = max(dot(N, Ldir), 0.0);\n"
    "    vec3 dirRadiance = dirLightCol * NdotLdir * 10.0;\n"
    "    vec3 Hdir = normalize(V + Ldir);\n"
    "    vec3 Fdir = FresnelSchlick(max(dot(Hdir, V), 0.0), F0);\n"
    "    vec3 kSdir = Fdir;\n"
    "    vec3 kDdir = (1.0 - kSdir) * (1.0 - metallic);\n"
    "    Lo += kDdir * albedo * dirRadiance + kSdir * dirRadiance;\n"

    "    vec3 ambient = vec3(0.05, 0.05, 0.08) * albedo * ao;\n"

    "    vec3 color = ambient + Lo;\n"

    "    float fogFactor = 1.0 - exp(-fogDensity * fogDensity * length(fragPosition) * length(fragPosition));\n"
    "    color = mix(color, fogColor, clamp(fogFactor, 0.0, 1.0));\n"

    "    color = color / (color + vec3(1.0));\n"
    "    color = pow(color, vec3(1.0 / 2.2));\n"
    "    finalColor = vec4(color, 1.0);\n"
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

    "float hash(vec2 p) {\n"
    "    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);\n"
    "}\n"

    "float paperGrain(vec2 uv) {\n"
    "    vec2 p = uv * resolution;\n"
    "    float n = hash(floor(p)) * 0.5 + hash(floor(p) + 0.5) * 0.5;\n"
    "    return (n - 0.5) * 0.07;\n"
    "}\n"

    "float sobelEdge(vec2 uv) {\n"
    "    float oX = 1.0 / resolution.x;\n"
    "    float oY = 1.0 / resolution.y;\n"

    "    vec3 tl = texture(texture0, uv + vec2(-oX,  oY)).rgb;\n"
    "    vec3 t  = texture(texture0, uv + vec2( 0.0,  oY)).rgb;\n"
    "    vec3 tr = texture(texture0, uv + vec2( oX,  oY)).rgb;\n"
    "    vec3 l  = texture(texture0, uv + vec2(-oX,  0.0)).rgb;\n"
    "    vec3 r  = texture(texture0, uv + vec2( oX,  0.0)).rgb;\n"
    "    vec3 bl = texture(texture0, uv + vec2(-oX, -oY)).rgb;\n"
    "    vec3 b  = texture(texture0, uv + vec2( 0.0, -oY)).rgb;\n"
    "    vec3 br = texture(texture0, uv + vec2( oX, -oY)).rgb;\n"

    "    float tlL = dot(tl, vec3(0.299, 0.587, 0.114));\n"
    "    float tL  = dot(t,  vec3(0.299, 0.587, 0.114));\n"
    "    float trL = dot(tr, vec3(0.299, 0.587, 0.114));\n"
    "    float lL  = dot(l,  vec3(0.299, 0.587, 0.114));\n"
    "    float rL  = dot(r,  vec3(0.299, 0.587, 0.114));\n"
    "    float blL = dot(bl, vec3(0.299, 0.587, 0.114));\n"
    "    float bL  = dot(b,  vec3(0.299, 0.587, 0.114));\n"
    "    float brL = dot(br, vec3(0.299, 0.587, 0.114));\n"

    "    float gx = -tlL + trL - 2.0 * lL + 2.0 * rL - blL + brL;\n"
    "    float gy = -tlL - 2.0 * tL - trL + blL + 2.0 * bL + brL;\n"

    "    return sqrt(gx * gx + gy * gy);\n"
    "}\n"

    "void main() {\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec3 col = texture(texture0, uv).rgb;\n"
    "    \n"
    "    float edge = sobelEdge(uv);\n"
    "    edge = smoothstep(0.12, 0.50, edge);\n"
    "    float inkMask = floor(edge * 4.0 + 0.5) / 4.0;\n"
    "    \n"
    "    vec3 paper = mix(vec3(0.98, 0.97, 0.94), vec3(0.95, 0.94, 0.90), uv.y);\n"
    "    paper += paperGrain(uv) * 0.03;\n"
    "    paper = clamp(paper, 0.0, 1.0);\n"
    "    \n"
    "    vec3 inkBlue = vec3(0.08, 0.18, 0.52);\n"
    "    vec3 inkCyan = vec3(0.06, 0.36, 0.60);\n"
    "    float inkMix = sin(uv.y * resolution.y * 0.4 + time * 0.3) * 0.5 + 0.5;\n"
    "    vec3 inkColor = mix(inkBlue, inkCyan, inkMix * 0.4);\n"
    "    \n"
    "    float lum = dot(col, vec3(0.299, 0.587, 0.114));\n"
    "    float paperMix = smoothstep(0.20, 0.80, lum) * 0.30;\n"
    "    vec3 result = mix(col, paper, paperMix);\n"
    "    \n"
    "    float darkLift = smoothstep(0.0, 0.18, lum);\n"
    "    result = mix(paper * 0.92, result, darkLift);\n"
    "    \n"
    "    result = mix(result, inkColor, inkMask * 0.75);\n"
    "    \n"
    "    float h1 = step(0.5, fract((uv.x + uv.y) * 52.0));\n"
    "    float h2 = step(0.5, fract((uv.x - uv.y) * 52.0));\n"
    "    float h3 = step(0.5, fract(uv.x * 52.0));\n"
    "    float hatch = 0.0;\n"
    "    if (lum > 0.40 && lum < 0.80) hatch = max(h1 * 0.85, max(h2 * 0.65, h3 * 0.45));\n"
    "    result = mix(result, inkColor * 0.85, hatch * 0.20);\n"
    "    \n"
    "    float lineSpacing = resolution.y / 18.0;\n"
    "    float lineY = fract(uv.y * resolution.y / lineSpacing);\n"
    "    float ruledLine = 1.0 - smoothstep(0.0, 0.02, lineY);\n"
    "    result = mix(result, inkColor * 0.6, ruledLine * 0.08);\n"
    "    \n"
    "    result = clamp(result, 0.0, 1.0);\n"
    "    finalColor = vec4(result, 1.0);\n"
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
