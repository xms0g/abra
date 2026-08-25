#ifndef PBR_GLSL
#define PBR_GLSL

#include "ub/light.glsl"
#include "ub/shadow.glsl"
#include "pbr/brdf.glsl"

#define ENVIROMENT_INTENSITY 0.5

// Shadow Map
uniform sampler2D shadowMap;
uniform samplerCubeArray shadowCubemap;
uniform sampler2DArray persShadowMap;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const vec3 gridSamplingDisk[20] = vec3[](
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float calculateDirectionalShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
float calculateOmnidirectionalShadow(vec3 fragWorldPos, vec3 normal, vec3 lightPos, vec3 cameraPos, int lightIndex);
float calculatePerspectiveShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int lightIndex);

vec3 calculateLights(vec3 N, vec3 V, vec3 R, vec3 fragWorldPos, vec3 albedo, float metallic, float roughness, float ao) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount.x; ++i) {
        DirectionalLight light = dirLights[i];
        vec3 lightDir = normalize(-light.direction.xyz);
        vec3 lightPos = lightDir * 5.0;
        vec3 radiance = light.diffuse.rgb * light.intensity.x;

        vec4 fragPosLightSpace = dirShadowData.lightSpaceMatrix * vec4(fragWorldPos, 1.0);
        float shadow = calculateDirectionalShadow(fragPosLightSpace, N, lightDir);

        Lo += brdf(lightPos, N, V, F0, fragWorldPos, radiance, albedo, metallic, roughness, ao) * (1.0 - shadow);
    }

    for (int i = 0; i < lightCount.y; ++i) {
        PointLight light = pointLights[i];
        vec3 lightPos = light.position.xyz;
        float distance = length(lightPos - fragWorldPos);
        float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
        vec3 radiance = light.diffuse.rgb * light.intensity.x * attenuation;

        float shadow = light.attenuation.w == 1.0 ? calculateOmnidirectionalShadow(fragWorldPos, N, lightPos, cameraPos.xyz, i) : 0.0;

        Lo += brdf(lightPos, N, V, F0, fragWorldPos, radiance, albedo, metallic, roughness, ao) * (1.0 - shadow);
    }

    for (int i = 0; i < lightCount.z; ++i) {
        SpotLight light = spotLights[i];
        vec3 lightPos = light.position.xyz;
        vec3 lightDir = normalize(lightPos - fragWorldPos);
        float distance = length(lightPos - fragWorldPos);
        float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
        // spotlight intensity
        float theta = dot(lightDir, normalize(-light.direction.xyz));
        float epsilon = light.cutOff.x - light.cutOff.y;
        float intensity = clamp((theta - light.cutOff.y) / epsilon, 0.0, 1.0);

        vec3 radiance = light.diffuse.rgb * light.cutOff.z * intensity * attenuation;

        vec4 fragPosPersLightSpace = perShadowData.lightSpaceMatrix[i] * vec4(fragWorldPos, 1.0);

        float shadow = light.attenuation.w == 1.0 ? calculatePerspectiveShadow(fragPosPersLightSpace, N, lightDir, i) : 0.0;

        Lo += brdf(lightPos, N, V, F0, fragWorldPos, radiance, albedo, metallic, roughness, ao) * (1.0 - shadow);
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;

    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 integratedBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * integratedBRDF.x + integratedBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * ao * ENVIROMENT_INTENSITY;
    vec3 color = ambient + Lo;

    return color;
}

float calculateDirectionalShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;// no shadow
    }
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

float calculateOmnidirectionalShadow(vec3 fragWorldPos, vec3 normal, vec3 lightPos, vec3 cameraPos, int lightIndex) {
    vec3 lightToFrag = fragWorldPos - lightPos;
    float currentDepth = length(lightToFrag);
    vec3 lightDir = normalize(lightPos - fragWorldPos);
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01);
    float viewDistance = length(cameraPos - fragWorldPos);
    float diskRadius = (1.0 + (viewDistance / omniShadowData.posFarPlane.w)) * 0.005;
    float shadow = 0.0;
    int samples = 20;

    for (int i = 0; i < samples; ++i) {
        vec3 sampleDir = normalize(lightToFrag + gridSamplingDisk[i] * diskRadius);
        float closestDepth = texture(shadowCubemap, vec4(sampleDir, float(lightIndex))).r;
        closestDepth *= omniShadowData.posFarPlane.w;// undo mapping [0;1]

        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }

    }
    shadow /= float(samples);

    return shadow;
}

float calculatePerspectiveShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int lightIndex) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // outside light frustum:
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    // simple bias based on normal and light direction (reduces peter-panning)
    float biasLocal = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    // PCF
    float shadow = 0.0;
    int samples = 2;
    vec2 texelSize = 1.0 / textureSize(persShadowMap, 0).xy;

    for (int x = -samples; x <= samples; ++x) {
        for (int y = -samples; y <= samples; ++y) {
            float pcfDepth = texture(persShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, lightIndex)).r;
            shadow += currentDepth - biasLocal > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= float((2 * samples + 1) * (2 * samples + 1));

    return shadow;
}

#endif

