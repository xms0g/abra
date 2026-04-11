#ifndef UB_LIGHT_GLSL
#define UB_LIGHT_GLSL

#include "common/constants.glsl"

struct DirectionalLight {
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec3 padding;
    float intensity;
};

struct PointLight {
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 attenuation;

    bool castShadow;

    vec3 padding;
    float intensity;
};

struct SpotLight {
    vec4 position;
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec3 attenuation;
    bool castShadow;

    vec3 cutOff;
    float intensity;
};

layout (std140) uniform LightBlock
{
    DirectionalLight dirLights[MAX_DIR_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
    ivec4 lightCount;
};

#endif
