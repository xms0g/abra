#version 410 core
#include "common/object.glsl"

out vec4 fragColor;

void main() {
    fragColor = texture(material.texture_diffuse, fs_in.TexCoord);
}