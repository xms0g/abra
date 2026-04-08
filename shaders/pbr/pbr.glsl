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
float calculateOmnidirectionalShadow(vec3 worldPos, vec3 normal, vec3 lightPos, vec3 viewPos, int lightIndex);

vec3 calculateLights(vec3 N, vec3 V, vec3 R, vec3 worldPos, vec4 fragPosLightSpace, vec3 albedo, float metallic, float roughness, float ao) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount.x; ++i) {
        DirectionalLight light = dirLights[i];
        vec3 lightDir = normalize(-light.direction.xyz);
        vec3 lightPos = lightDir * 5.0;
        vec3 radiance = light.diffuse.rgb * light.intensity;

        float shadow = calculateDirectionalShadow(fragPosLightSpace, N, lightDir);

        Lo += brdf(lightPos, N, V, F0, worldPos, radiance, albedo, metallic, roughness, ao) * (1.0 - shadow);
    }

    for (int i = 0; i < lightCount.y; ++i) {
        PointLight light = pointLights[i];
        vec3 lightPos = light.position.xyz;
        float distance = length(lightPos - worldPos);
        float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
        vec3 radiance = light.diffuse.rgb * light.intensity * attenuation;

        float shadow = calculateOmnidirectionalShadow(worldPos, N, lightPos, viewPos.xyz, i);

        Lo += brdf(lightPos, N, V, F0, worldPos, radiance, albedo, metallic, roughness, ao) * (1.0 - shadow);
    }

    for (int i = 0; i < lightCount.z; i++) {
        // Lo += brdf(spotLights[i].position.xyz, worldPos, spotLights[i].diffuse.rgb, albedo, N, metallic, roughness, ao, V, F0);
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
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

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
    int samples = 2;
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

float calculateOmnidirectionalShadow(vec3 worldPos, vec3 normal, vec3 lightPos, vec3 viewPos, int lightIndex) {
    vec3 fragToLight = worldPos - lightPos.xyz;
    float currentDepth = length(fragToLight);
    vec3 lightDir = normalize(worldPos - lightPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float viewDistance = length(viewPos - worldPos);
    float diskRadius = (1.0 + (viewDistance / omniFarPlanes.x)) * 0.005;
    float shadow = 0.0;
    int samples = 20;

    for (int i = 0; i < samples; ++i) {
        vec3 sampleDir = normalize(fragToLight + gridSamplingDisk[i] * diskRadius);
        float closestDepth = texture(shadowCubemap, vec4(sampleDir, float(lightIndex))).r;
        closestDepth *= omniFarPlanes.x;// undo mapping [0;1]

        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }

    }
    shadow /= float(samples);

    return shadow;
}

