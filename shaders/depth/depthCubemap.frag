#version 410 core
in vec4 FragPos;

#include "ub/shadow.glsl"

void main() {
    float lightDistance = length(FragPos.xyz - omniShadowData.posFarPlane.xyz);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / omniShadowData.posFarPlane.w;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}