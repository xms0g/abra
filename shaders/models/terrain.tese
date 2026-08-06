#version 410 core
layout (quads, fractional_odd_spacing, cw) in;

#define HEIGHT_SHIFT 16.0

in TCS_OUT
{
    vec2 TexCoord;
} tes_in[];

#include "ub/camera.glsl"
#include "legacy/material.glsl"

uniform mat4 model;

out TES_OUT
{
    float Height;
} tes_out;

void main() {
    // get patch coordinate
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // retrieve control point texture coordinates
    vec2 t00 = tes_in[0].TexCoord;
    vec2 t01 = tes_in[1].TexCoord;
    vec2 t10 = tes_in[2].TexCoord;
    vec2 t11 = tes_in[3].TexCoord;

    // bilinearly interpolate texture coordinate across patch
    vec2 t0 = mix(t00, t01, u);
    vec2 t1 = mix(t10, t11, u);
    vec2 texCoord = mix(t0, t1, v);

    // lookup texel at patch coordinate for height and scale + shift as desired
    tes_out.Height = texture(material.texture_height, texCoord).y * material.heightScale - HEIGHT_SHIFT;

    // retrieve control point position coordinates
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // compute patch surface normal
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize(vec4(cross(vVec.xyz, uVec.xyz), 0));

    // bilinearly interpolate position coordinate across patch
    vec4 p0 = mix(p00, p01, u);
    vec4 p1 = mix(p10, p11, u);
    vec4 p = mix(p0, p1, v);

    p += normal * tes_out.Height;

    gl_Position = projection * view * model * p;
}