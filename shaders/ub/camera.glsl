#ifndef UB_CAMERA_GLSL
#define UB_CAMERA_GLSL

layout (std140) uniform CameraBlock
{
    mat4 view;
    mat4 skyView;
    vec4 viewPos;
    mat4 projection;
    mat4 inverseProjection;
};

#endif