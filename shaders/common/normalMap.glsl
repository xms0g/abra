#ifndef COMMON_NORMAL_MAP_GLSL
#define COMMON_NORMAL_MAP_GLSL

vec3 normal(mat3 TBN, vec2 texCoord, bool hasNormalMap) {
    if (!hasNormalMap)
        return TBN[2];

    vec3 n = texture(material.texture_normal, texCoord).xyz;
    // transform normal vector to range [-1,1]
    n = n * 2.0 - 1.0;
    // Transform N Tangent space to World space
    n = normalize(TBN * n);

    return n;
}

#endif
