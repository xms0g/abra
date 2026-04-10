struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_roughnessMetallic;
    sampler2D texture_ao; // optional
    sampler2D texture_emissive;
    sampler2D texture_height;

    float alphaCutoff;
    float heightScale;

    bool hasEmissiveMap;
    bool hasHeightMap;
    bool hasAOMap;
    bool hasORM;
};

uniform Material material;