#version 410 core
layout (location = 0) in vec3 aPos;

#include "ub/camera.glsl"

out VS_OUT
{
    vec3 TexCoord;
} vs_out;

void main() {
    vs_out.TexCoord = aPos;
    vec4 pos = projection * skyView * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
