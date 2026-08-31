#ifndef COMMON_TBN_GLSL
#define COMMON_TBN_GLSL

mat3 TBN(vec4 tangent, vec3 normal) {
    vec3 T = normalize(tangent.xyz);
    vec3 N = normalize(normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    return mat3(T, B, N);
}

#endif
