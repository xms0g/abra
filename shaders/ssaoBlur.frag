#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

uniform sampler2D ssaoTexture;

out vec4 fragColor;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTexture, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoTexture, fs_in.TexCoord + offset).r;
        }
    }
    fragColor = vec4(vec3(result / (4.0 * 4.0)), 1.0);
}
