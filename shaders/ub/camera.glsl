layout (std140) uniform CameraBlock
{
    mat4 view;
    vec4 viewPos;
    mat4 projection;
    mat4 inverseProjection;
};