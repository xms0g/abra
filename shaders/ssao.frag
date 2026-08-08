#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

#include "ub/camera.glsl"

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

layout (std140) uniform SSAOBlock
{
    vec4 samples[16];
    vec4 settings;
    vec4 resolution;
};

out vec4 fragColor;

void main() {
    float radius = settings.x;
    float bias = settings.y;
    float intensity = settings.z;
    int kernelSize = int(settings.w);
    vec2 noiseScale = resolution.zw;
    // get input for SSAO algorithm
    vec3 fragPosView = texture(gPosition, fs_in.TexCoord).xyz;
    vec3 normalView = texture(gNormal, fs_in.TexCoord).xyz;
    vec3 randomVec = texture(texNoise, fs_in.TexCoord * noiseScale).xyz;
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normalView * dot(randomVec, normalView));
    vec3 bitangent = cross(normalView, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalView);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        // get sample position
        vec3 samplePos = TBN * samples[i].xyz;// from tangent to view-space
        samplePos = fragPosView + samplePos * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;// from view to clip-space
        offset.xyz /= offset.w;// perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;// transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z;

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosView.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    occlusion = pow(occlusion, intensity);

    fragColor = vec4(vec3(occlusion), 1.0);
}
