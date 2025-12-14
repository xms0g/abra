#version 410 core
#include "common/object.glsl"

out vec4 fragColor;

void main() {
    float alpha = texture(material.texture_diffuse, fs_in.TexCoord).a;
    if (alpha < material.alphaCutout) {
        discard;
    }

    vec2 texCoord = parallaxMapping(fs_in.TexCoord, fs_in.TangentViewDir, material.heightScale, material.hasHeightMap);
    vec3 normal = normalMapping(fs_in.TBN, texCoord, material.hasNormalMap);
    vec3 diffuse = texture(material.texture_diffuse, texCoord).rgb;
    float specular = texture(material.texture_specular, texCoord).r;
    // Create a mask: 0.0 if no lights, 1.0 if at least one light
    bool hasLights = lightCount.x > 0 || lightCount.y > 0 || lightCount.z > 0;
    vec3 result = mix(diffuse, calculateLights(normal, fs_in.FragPos, viewPos.xyz, fs_in.ViewDir, fs_in.FragPosLightSpace, diffuse, specular, material.shininess), float(hasLights));

    fragColor = vec4(result, 1.0);
}