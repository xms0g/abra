#version 410 core
in VS_OUT
{
    vec2 TexCoord;
} fs_in;

#include "ub/camera.glsl"

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform int kernelSize;

layout (std140) uniform SSAOBlock
{
    vec4 samples[32];
    vec4 rbi;
    vec4 resolution;
};

out vec4 fragColor;

void main() {
    float radius = rbi.x;
    float bias = rbi.y;
    float intensity = rbi.z;
    vec2 noiseScale = vec2(resolution.x/4.0, resolution.y/4.0);
    // get input for SSAO algorithm
    vec3 normalWorld = texture(gNormal, fs_in.TexCoord).xyz;
    vec3 fragPosView = texture(gPosition, fs_in.TexCoord).xyz;
    vec3 normalView = normalize(mat3(view) * normalWorld);

    vec3 randomVec = normalize(texture(texNoise, fs_in.TexCoord * noiseScale).rgb);
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
