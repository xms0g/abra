#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out VS_OUT
{
    vec3 WorldPos;
} vs_out;

void main() {
    vs_out.WorldPos = aPos;
    gl_Position = projection * view * vec4(vs_out.WorldPos, 1.0);
}
