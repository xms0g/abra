#version 410 core
#include "legacy/object.glsl"

out vec4 fragColor;

void main() {
    vec2 texCoord = parallaxMapping(fs_in.TexCoord, fs_in.TangentViewDir, material.heightScale, material.hasHeightMap);
    vec3 N = normal(fs_in.TBN, texCoord, false);
    vec3 diffuse = texture(material.texture_albedo, texCoord).rgb;
    vec3 result = calculateLights(N, fs_in.FragPos, viewPos.xyz, fs_in.ViewDir, fs_in.FragPosLightSpace, diffuse, 1.0, 32.0, 1.0);

    fragColor = vec4(result, 1.0);
}