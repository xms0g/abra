#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

#include "ub/camera.glsl"
#include "ub/shadow.glsl"
#include "pbr/pbr.glsl"

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gORM;
uniform sampler2D ssao;

out vec4 fragColor;

void main() {
    vec4 positionSample = texture(gPosition, fs_in.TexCoord);
    vec4 normalSample = texture(gNormal, fs_in.TexCoord);
    vec3 fragWorldPos = positionSample.xyz;
    vec3 N = normalSample.xyz;
    vec3 V = normalize(cameraPos.xyz - fragWorldPos);
    vec3 R = reflect(-V, N);

    vec4 albedoSample = texture(gAlbedo, fs_in.TexCoord);
    vec3 albedo = albedoSample.rgb;

    vec3 orm = texture(gORM, fs_in.TexCoord).rgb;
    float metallic = orm.b;
    float roughness = orm.g;
    float bakedAO = orm.r;

    float finalAO = texture(ssao, fs_in.TexCoord).r * bakedAO;

    vec3 emissive = vec3(positionSample.w, normalSample.w, albedoSample.a);

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragWorldPos, 1.0);

    vec3 result = calculateLights(N, V, R, fragWorldPos, fragPosLightSpace, albedo, metallic, roughness, finalAO) + emissive;

    fragColor = vec4(result, 1.0);
}