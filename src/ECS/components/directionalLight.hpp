#pragma once
#include "glm/glm.hpp"

struct DirectionalLightComponent {
	uint32_t idx{};
	glm::vec3 direction{};
	glm::vec3 ambient{};
	glm::vec3 diffuse{};
	glm::vec3 specular{};
	float intensity{};

	DirectionalLightComponent() = default;

	explicit DirectionalLightComponent(
		const uint32_t i,
		const glm::vec3 dir,
		const glm::vec3 a,
		const glm::vec3 dif,
		const glm::vec3 s,
		const float intens)
		: idx(i),
		  direction(dir),
		  ambient(a),
		  diffuse(dif),
		  specular(s),
		  intensity(intens) {
	}
};
