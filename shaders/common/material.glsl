const int ALPHA_OPAQUE = 1 << 0;
const int ALPHA_BLEND = 1 << 1;
const int ALPHA_CUTOUT  = 1 << 2;

struct Material {
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    sampler2D texture_normal;
    sampler2D texture_height;
    vec3 color;
    float shininess;
    float heightScale;
    int mode;
    float alphaCutout;
    bool hasNormalMap;
    bool hasHeightMap;
};

uniform Material material;