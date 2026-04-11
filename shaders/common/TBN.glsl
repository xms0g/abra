#ifndef COMMON_TBN_GLSL
#define COMMON_TBN_GLSL

mat3 TBN(mat4 model, vec3 tangent, mat3 normalMatrix, vec3 normal) {
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 N = normalize(normalMatrix * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    return mat3(T, B, N);
}

#endif
