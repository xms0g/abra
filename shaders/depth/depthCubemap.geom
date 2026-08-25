#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

#include "ub/shadow.glsl"

uniform int cubeIndex;

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = cubeIndex * 6 + face; // built-in variable that specifies to which face we render.

        for (int i = 0; i < 3; ++i) {
            FragPos = gl_in[i].gl_Position;
            gl_Position = omniShadowData.shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}