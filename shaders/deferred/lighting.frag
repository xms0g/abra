#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

#include "ub/camera.glsl"
#include "ub/shadow.glsl"
#include "common/blinnPhong.glsl"

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

out vec4 fragColor;

void main() {
    vec3 fragPos = texture(gPosition, fs_in.TexCoord).xyz;
    vec3 normal = texture(gNormal, fs_in.TexCoord).xyz;
    vec3 diffuse = texture(gAlbedoSpec, fs_in.TexCoord).rgb;
    float specular = texture(gAlbedoSpec, fs_in.TexCoord).a;
    float ambientOcclusion = texture(ssao, fs_in.TexCoord).r;
    vec3 viewDir = normalize(viewPos.xyz - fragPos);
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    // Create a mask: 0.0 if no lights, 1.0 if at least one light
    bool hasLights = lightCount.x > 0 || lightCount.y > 0 || lightCount.z > 0;
    vec3 result = hasLights ? calculateLights(normal, fragPos, viewPos.xyz, viewDir, fragPosLightSpace, diffuse, specular, 32.0, ambientOcclusion) : diffuse;

    fragColor = vec4(result, 1.0);
}