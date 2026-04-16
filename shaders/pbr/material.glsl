#ifndef PBR_MATERIAL_GLSL
#define PBR_MATERIAL_GLSL

#define HAS_HEIGHT_MAP      (1u << 6)
#define HAS_EMISSIVE_MAP    (1u << 7)
#define HAS_AO_MAP          (1u << 8)
#define HAS_ORM             (1u << 9)

struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_roughnessMetallic;
    sampler2D texture_ao; // optional
    sampler2D texture_emissive;
    sampler2D texture_height;

    float alphaCutoff;
    float heightScale;
    uint flags;
};

uniform Material material;

#endif