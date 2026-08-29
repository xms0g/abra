#version 410 core

out VS_OUT
{
    vec2 TexCoord;
} vs_out;

void main() {
    vec2 coords[3] = vec2[3](
        vec2(-1.0f, -1.0f),
        vec2(3.0f, -1.0f),
        vec2(-1.0f, 3.0f)
    );

    vs_out.TexCoord = coords[gl_VertexID] * 0.5 + 0.5;
    gl_Position = vec4(coords[gl_VertexID], 0.0f, 1.0f);
}