#version 410 core
#include "legacy/object.glsl"

out vec4 fragColor;

void main() {
    vec2 texCoord = parallaxMapping(fs_in.TexCoord, fs_in.TangentViewDir, material.heightScale, false);
    vec3 N = normal(fs_in.TBN, fs_in.TexCoord, false);
    vec3 diffuse = texture(material.texture_albedo, fs_in.TexCoord).rgb;
    float specular = texture(material.texture_specular, fs_in.TexCoord).r;
    vec3 result = calculateLights(N, fs_in.WorldPos, cameraPos.xyz, fs_in.ViewDir, fs_in.FragPosLightSpace, diffuse, specular, 32.0, 1.0);

    fragColor = vec4(result, 1.0);
}