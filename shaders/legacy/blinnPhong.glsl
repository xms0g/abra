#ifndef LEGACY_PHONG_GLSL
#define LEGACY_PHONG_GLSL

#include "ub/light.glsl"

uniform sampler2D shadowMap;
uniform samplerCubeArray shadowCubemap;
uniform sampler2DArray persShadowMap;

const vec3 gridSamplingDisk[20] = vec3[](
    vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
    vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
    vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
    vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace, vec3 albedo, float specular, float shininess, float ao);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewPos,vec3 viewDir, vec3 albedo, float specular, float shininess, float ao, int layer);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 fragPosLightSpace, vec3 albedo, float specular, float shininess, float ao, int layer);

vec3 calculateLights(vec3 normal, vec3 fragPos, vec3 viewPos, vec3 viewDir, vec4 fragPosLightSpace, vec3 albedo, float specular, float shininess, float ao) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < lightCount.x; i++) {
        result += calculateDirectionalLight(dirLights[i], normal, viewDir, fragPosLightSpace, albedo, specular, shininess, ao);
    }
    for (int i = 0; i < lightCount.y; i++) {
        result += calculatePointLight(pointLights[i], normal, fragPos, viewPos, viewDir, albedo, specular, shininess, ao, i);
    }
    for (int i = 0; i < lightCount.z; i++) {
        vec4 fragPosPersLightSpace = persLightSpaceMatrix[i] * vec4(fragPos, 1.0);
        result += calculateSpotLight(spotLights[i], normal, fragPos, viewDir, fragPosPersLightSpace, albedo, specular, shininess, ao, i);
    }

    return result;
}

float calculateDirectionalShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (projCoords.z > 1.0)
        return 0.0;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // PCF
    float shadow = 0.0;
    int samples = 5;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -samples; x <= samples; ++x) {
        for (int y = -samples; y <= samples; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= float((2 * samples + 1) * (2 * samples + 1));

    return shadow;
}

float calculateOmnidirectionalShadow(vec3 fragPos, vec4 lightPos, vec3 viewPos, int layer) {
    vec3 fragToLight = fragPos - lightPos.xyz;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / omniFarPlane.x)) / 25.0;

    for (int i = 0; i < samples; ++i) {
        vec3 sampleDir = normalize(fragToLight + gridSamplingDisk[i] * diskRadius);
        float closestDepth = texture(shadowCubemap, vec4(sampleDir, float(layer))).r;
        closestDepth *= omniFarPlane.x;   // undo mapping [0;1]

        if (currentDepth - bias > closestDepth)
        shadow += 1.0;
    }
    shadow /= float(samples);

    return shadow;
}

float calculatePerspectiveShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int layer) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    // outside light frustum:
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
    projCoords.y < 0.0 || projCoords.y > 1.0 ||
    projCoords.z > 1.0)
    return 0.0;
    // simple bias based on normal and light direction (reduces peter-panning)
    float biasLocal = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    // PCF
    float shadow = 0.0;
    int samples = 2;
    vec2 texelSize = 1.0 / textureSize(persShadowMap, 0).xy;
    for (int x = -samples; x <= samples; ++x) {
        for (int y = -samples; y <= samples; ++y) {
            float pcfDepth = texture(persShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += currentDepth - biasLocal > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= float((2 * samples + 1) * (2 * samples + 1));

    return shadow;
}



vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace, vec3 albedo, float specular, float shininess, float ao) {
    vec3 lightDir = normalize(-light.direction.xyz);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    // combine results
    vec3 ambient = light.ambient.rgb * albedo * ao;
    vec3 diffuse = light.diffuse.rgb * diff * albedo;
    vec3 finalSpecular = light.specular.rgb * spec * specular;

    // calculate shadow
    float shadow = calculateDirectionalShadow(fragPosLightSpace, normal, lightDir);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + finalSpecular));
    return lighting;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewPos,vec3 viewDir, vec3 albedo, float specular, float shininess, float ao, int layer) {
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // attenuation
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));

    // combine results
    vec3 ambient = light.ambient.rgb * albedo * ao;
    vec3 diffuse = light.diffuse.rgb * diff * albedo;
    vec3 finalSpecular = light.specular.rgb * spec * specular;

    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = light.attenuation.w == 1.0 ? calculateOmnidirectionalShadow(fragPos, light.position, viewPos, layer) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + finalSpecular));
    return lighting;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 fragPosLightSpace, vec3 albedo, float specular, float shininess, float ao, int layer) {
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // attenuation
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.cutOff.x - light.cutOff.y;
    float intensity = clamp((theta - light.cutOff.y) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient.rgb * albedo * ao;
    vec3 diffuse = light.diffuse.rgb * diff * albedo;
    vec3 finalSpecular = light.specular.rgb * spec * specular;

    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    float shadow =  light.attenuation.w == 1.0 ? calculatePerspectiveShadow(fragPosLightSpace, normal, lightDir, layer) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + finalSpecular));
    return lighting;
}

#endif