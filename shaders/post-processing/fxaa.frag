#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

#define FXAA_REDUCE_MIN (1.0/16.0)
#define FXAA_REDUCE_MUL (1.0/4.0)
#define FXAA_SPAN_MAX 8.0

uniform sampler2D screenTexture;
uniform vec2 inverseResolution;

out vec4 fragColor;

void main() {
    vec3 upperLeft = texture(screenTexture, fs_in.TexCoord + (vec2(-1.0, -1.0) * inverseResolution)).rgb;
    vec3 upperRight = texture(screenTexture, fs_in.TexCoord + (vec2(1.0, -1.0) * inverseResolution)).rgb;
    vec3 lowerLeft = texture(screenTexture, fs_in.TexCoord + (vec2(-1.0, 1.0) * inverseResolution)).rgb;
    vec3 lowerRight = texture(screenTexture, fs_in.TexCoord + (vec2(1.0, 1.0) * inverseResolution)).rgb;
    vec3 middle  = texture(screenTexture, fs_in.TexCoord).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaUL = dot(upperLeft, luma);
    float lumaUR = dot(upperRight, luma);
    float lumaLL = dot(lowerLeft, luma);
    float lumaLR = dot(lowerRight, luma);
    float lumaMiddle = dot(middle,  luma);

    float lumaMin = min(lumaMiddle, min(min(lumaUL, lumaUR), min(lumaLL, lumaLR)));
    float lumaMax = max(lumaMiddle, max(max(lumaUL, lumaUR), max(lumaLL, lumaLR)));

    vec2 dir;
    dir.x = -((lumaUL + lumaUR) - (lumaLL + lumaLR));
    dir.y =  ((lumaUL + lumaLL) - (lumaUR + lumaLR));

    float dirReduce = max((lumaUL + lumaUR + lumaLL + lumaLR) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(
        vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
        max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseResolution;

    vec3 rgbA = (1.0/2.0) * (
        texture(screenTexture, fs_in.TexCoord.xy + dir * (1.0/3.0)).xyz +
        texture(screenTexture, fs_in.TexCoord.xy + dir * (2.0/3.0)).xyz);

    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture(screenTexture, fs_in.TexCoord.xy + dir * (-1.0/3.0)).xyz +
        texture(screenTexture, fs_in.TexCoord.xy + dir * (-2.0/3.0)).xyz);

    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) {
        fragColor = vec4(rgbA, 1.0);
    } else {
        fragColor = vec4(rgbB, 1.0);
    }
}
