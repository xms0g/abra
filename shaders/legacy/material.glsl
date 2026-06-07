#ifndef LEGACY_MATERIAL_GLSL
#define LEGACY_MATERIAL_GLSL

struct Material {
    sampler2D texture_albedo;
    sampler2D texture_specular;
    sampler2D texture_normal;
    sampler2D texture_height;

    vec3 color;
    float heightScale;
    float alphaCutout;
    uint flags;
};

uniform Material material;

#endif