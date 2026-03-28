#version 410 core
in VS_OUT
{
    vec3 WorldPos;
} fs_in;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 sampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

out vec4 fragColor;

void main() {
    vec2 uv = sampleSphericalMap(normalize(fs_in.WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    fragColor = vec4(color, 1.0);
}
