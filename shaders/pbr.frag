#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    vec3 Normal;
} fs_in;


#include "ub/camera.glsl"
#include "common/brdf.glsl"

struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_metallic;
    sampler2D texture_roughness;
    sampler2D texture_ao;
};

uniform Material material;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(material.texture_normal, fs_in.TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.WorldPos);
    vec3 Q2  = dFdy(fs_in.WorldPos);
    vec2 st1 = dFdx(fs_in.TexCoord);
    vec2 st2 = dFdy(fs_in.TexCoord);

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

out vec4 fragColor;

void main() {
    vec3 albedo = pow(texture(material.texture_albedo, fs_in.TexCoord).rgb, vec3(2.2));
    float metallic = texture(material.texture_metallic, fs_in.TexCoord).r;
    float roughness = texture(material.texture_roughness, fs_in.TexCoord).r;
    float ao = texture(material.texture_ao, fs_in.TexCoord).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos.xyz - fs_in.WorldPos);

    vec3 result = calculateLights(albedo, N, metallic, roughness, ao, V, fs_in.WorldPos);

    fragColor = vec4(result, 1.0);
}