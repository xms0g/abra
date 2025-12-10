#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

uniform sampler2D screenTexture;
uniform float intensity;

out vec4 fragColor;

void main() {
    float r = texture(screenTexture, fs_in.TexCoord - vec2(intensity, intensity)).r;
    float g = texture(screenTexture, fs_in.TexCoord ).g;
    float b = texture(screenTexture, fs_in.TexCoord + vec2(intensity, intensity)).b;

    fragColor = vec4(r, g, b, 1.0);
}
