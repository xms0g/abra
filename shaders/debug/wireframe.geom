#version 410 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

#include "ub/camera.glsl"

out GS_OUT {
    vec3 Color;
} gs_out;

void emitEdge(int i0, int i1, vec3 color) {
    vec4 p0 = gl_in[i0].gl_Position;
    vec4 p1 = gl_in[i1].gl_Position;

    gl_Position = projection * p0;
    gs_out.Color = color;
    EmitVertex();

    gl_Position = projection * p1;
    gs_out.Color = color;
    EmitVertex();

    EndPrimitive();
}

void main() {
    // color edges uniformly (or choose per-edge)
    vec3 color = vec3(0.0, 1.0, 0.0); // orange wireframe

    // edges: 0-1, 1-2, 2-0
    emitEdge(0, 1, color);
    emitEdge(1, 2, color);
    emitEdge(2, 0, color);
}