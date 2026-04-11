#ifndef UB_SHADOW_GLSL
#define UB_SHADOW_GLSL

#include "common/constants.glsl"

layout (std140) uniform ShadowBlock
{
    mat4 lightSpaceMatrix;
    mat4 persLightSpaceMatrix[MAX_SPOT_LIGHTS];
    vec4 omniFarPlanes;
};

#endif
