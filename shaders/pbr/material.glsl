struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_metallic;
    sampler2D texture_roughness;
    sampler2D texture_emissive;
    sampler2D texture_ao;
    bool hasEmissiveMap;
};

uniform Material material;