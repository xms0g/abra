#ifndef UB_CAMERA_GLSL
#define UB_CAMERA_GLSL

layout (std140) uniform CameraBlock
{
    mat4 view;
    mat4 inverseView;
    mat4 skyView;
    vec4 cameraPos;
    mat4 projection;
};

#endif