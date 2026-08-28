#version 410 core
in vec4 FragPos;

#include "ub/shadow.glsl"

void main() {
    vec3 ligthPos = omniShadowData.posFarPlane.xyz;
    float farPlane = omniShadowData.posFarPlane.w;

    float lightDistance = length(FragPos.xyz - ligthPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}