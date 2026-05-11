#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

uniform sampler2D screenTexture;

out vec4 fragColor;

void main() {
    vec3 color = texture(screenTexture, fs_in.TexCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    fragColor = brightness > 1.0 ? vec4(color, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}
