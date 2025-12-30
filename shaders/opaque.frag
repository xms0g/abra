#version 410 core
#include "common/object.glsl"

out vec4 fragColor;

void main() {
    vec2 texCoord = parallaxMapping(fs_in.TexCoord, fs_in.TangentViewDir, material.heightScale, material.hasHeightMap);
    vec3 normal = normalMapping(fs_in.TBN, texCoord, material.hasNormalMap);
    vec3 diffuse = material.hasDiffuseMap ? texture(material.texture_diffuse, texCoord).rgb : material.color;
    float specular = material.hasSpecularMap ? texture(material.texture_specular, texCoord).r : 0.0;
    // Create a mask: 0.0 if no lights, 1.0 if at least one light
    bool hasLights = lightCount.x > 0 || lightCount.y > 0 || lightCount.z > 0;
    vec3 result = hasLights ? calculateLights(normal, fs_in.FragPos, viewPos.xyz, fs_in.ViewDir, fs_in.FragPosLightSpace, diffuse, specular, material.shininess, 1.0) : diffuse;

    fragColor = vec4(result, 1.0);
}