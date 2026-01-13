#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 FragPos;
    mat3 TBN;
    vec3 TangentViewDir;
} fs_in;

#include "common/material.glsl"
#include "common/normalMap.glsl"
#include "common/parallaxMap.glsl"

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

void main() {
    // store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;
    vec2 texCoord = parallaxMapping(fs_in.TexCoord, fs_in.TangentViewDir, material.heightScale, material.hasHeightMap);
    // also store the per-fragment normals into the gbuffer
    gNormal = normal(fs_in.TBN, texCoord, material.hasNormalMap);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.texture_albedo, texCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = material.hasSpecularMap ? texture(material.texture_specular, texCoord).r : 0.0;
}
