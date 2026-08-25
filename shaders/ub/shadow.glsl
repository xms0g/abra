#ifndef UB_SHADOW_GLSL
#define UB_SHADOW_GLSL

#include "common/constants.glsl"

struct DirectionalShadowData {
    mat4 lightSpaceMatrix;
};

struct OmnidirectionalShadowData {
    vec4 omniFarPlane;
};

struct PerspectiveShadowData {
    mat4 lightSpaceMatrix[MAX_SPOT_LIGHTS];
};

layout (std140) uniform ShadowBlock
{
    DirectionalShadowData dirShadowData;
    OmnidirectionalShadowData omniShadowData;
    PerspectiveShadowData perShadowData;
};

#endif
