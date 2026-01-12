#version 410 core
#include "common/object.glsl"

out vec4 fragColor;

void main() {
    fragColor = texture(material.texture_albedo, fs_in.TexCoord);
}