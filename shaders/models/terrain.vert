#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out VS_OUT
{
    vec2 TexCoord;
} vs_out;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vs_out.TexCoord = aTex;
}
