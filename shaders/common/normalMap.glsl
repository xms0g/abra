vec3 normalMapping(mat3 TBN, vec2 texCoord, bool hasNormalMap) {
    if (!hasNormalMap)
        return TBN[2];

    vec3 normal = texture(material.texture_normal, texCoord).xyz;
    // transform normal vector to range [-1,1]
    normal = normal * 2.0 - 1.0;
    // Transform N Tangent space to World space
    normal = normalize(TBN * normal);

    return normal;
}
