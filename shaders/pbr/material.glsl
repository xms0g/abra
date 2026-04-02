struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_metallicRoughness;
    sampler2D texture_emissive;
    sampler2D texture_ao;
    bool hasEmissiveMap;
};

uniform Material material;