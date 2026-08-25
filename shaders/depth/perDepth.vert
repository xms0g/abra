#version 410 core
layout (location = 0) in vec3 aPos;

#include "ub/camera.glsl"
#include "ub/shadow.glsl"

uniform mat4 model;
uniform int shadowIndex;

void main() {
    gl_Position = perShadowData.lightSpaceMatrix[shadowIndex] * model * vec4(aPos, 1.0);
}