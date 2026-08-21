#version 410 core
in VS_OUT
{
    vec2 TexCoord;
    vec3 WorldPos;
    mat3 TBN;
    vec3 TangentViewDir;
} fs_in;

#include "ub/camera.glsl"
#include "pbr/material.glsl"
#include "common/normalMap.glsl"
#include "common/parallaxMap.glsl"

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gORM;

void main() {
    vec2 texCoord = parallaxMapping(fs_in.TexCoord, fs_in.TangentViewDir, material.heightScale, (material.flags & HAS_HEIGHT_MAP) != 0);

    vec4 albedoSample = texture(material.texture_albedo, texCoord);
    if (albedoSample.a < material.alphaCutoff) {
        discard;
    }

    // store the fragment position vector in the first gbuffer texture
    gPosition.rgb = fs_in.WorldPos;
    // also store the per-fragment normals into the gbuffer
    gNormal.rgb = normal(fs_in.TBN, texCoord, true);
    // and the diffuse per-fragment color
    gAlbedo.rgb = albedoSample.rgb;
    // Occulusion - Roughness - Metallic
    vec3 orm = texture(material.texture_roughnessMetallic, texCoord).rgb;
    float ao = 1.0;
    if ((material.flags & HAS_ORM) != 0) {
        ao = orm.r;
    } else if ((material.flags & HAS_AO_MAP) != 0) {
        ao = texture(material.texture_ao, texCoord).r;
    }
    gORM = vec3(ao, orm.g, orm.b);
    // Emissive
    vec3 emissive = vec3(0.0);
    if ((material.flags & HAS_EMISSIVE_MAP) != 0) {
        emissive = texture(material.texture_emissive, texCoord).rgb;
    }

    gPosition.w = emissive.r;
    gNormal.w = emissive.g;
    gAlbedo.a = emissive.b;
}
