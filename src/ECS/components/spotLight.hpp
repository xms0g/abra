#pragma once
#include "glm/glm.hpp"

struct SpotLightComponent {
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float cutOff;
	float outerCutOff;
	float intensity;
	bool castShadow;

	SpotLightComponent() = default;

	explicit SpotLightComponent(
		const glm::vec3 pos,
		const glm::vec3 dir,
		const glm::vec3 a,
		const glm::vec3 dif,
		const glm::vec3 s,
		const float kc,
		const float kl,
		const float kq,
		const float co,
		const float oc,
		const float i,
		const bool cs)
		: position(pos),
		  direction(dir),
		  ambient(a),
		  diffuse(dif),
		  specular(s),
		  constant(kc),
		  linear(kl),
		  quadratic(kq),
		  cutOff(co),
		  outerCutOff(oc),
		  intensity(i),
		  castShadow(cs) {
	}
};
