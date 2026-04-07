#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

uniform sampler2D hdrTexture;
uniform float exposure;

out vec4 fragColor;

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 hdrColor = texture(hdrTexture, fs_in.TexCoord).rgb;
    vec3 result = ACESFilm(hdrColor * exposure);
    fragColor = vec4(result, 1.0);
}