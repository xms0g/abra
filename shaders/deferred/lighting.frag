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
uniform sampler2D gEmissive;
uniform sampler2D ssao;

out vec4 fragColor;

void main() {
    vec3 worldPos = texture(gPosition, fs_in.TexCoord).xyz;
    vec3 N = texture(gNormal, fs_in.TexCoord).xyz;
    vec3 V = normalize(viewPos.xyz - worldPos);
    vec3 R = reflect(-V, N);

    vec3 albedo = texture(gAlbedo, fs_in.TexCoord).rgb;

    vec3 orm = texture(gORM, fs_in.TexCoord).rgb;
    float metallic = orm.b;
    float roughness = orm.g;
    float bakedAO = orm.r;

    float finalAO = texture(ssao, fs_in.TexCoord).r * bakedAO;

    vec3 emissive = texture(gEmissive, fs_in.TexCoord).rgb;

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);

    vec3 result = calculateLights(N, V, R, worldPos, fragPosLightSpace, albedo, metallic, roughness, finalAO) + emissive;

    fragColor = vec4(result, 1.0);
}