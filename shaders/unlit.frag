#version 410 core
#include "legacy/material.glsl"

out vec4 fragColor;

void main() {
    fragColor = vec4(material.color, 1.0);
}
