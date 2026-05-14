#pragma once
#include "glm/glm.hpp"

struct SpotLightComponent {
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 attenuation; // (Kc, Kl, kq)
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
		const glm::vec3 att,
		const float co,
		const float oc,
		const float i,
		const bool cs)
		: position(pos),
		  direction(dir),
		  ambient(a),
		  diffuse(dif),
		  specular(s),
		  attenuation(att),
		  cutOff(co),
		  outerCutOff(oc),
		  intensity(i),
		  castShadow(cs) {
	}
};
