#pragma once
#include "glm/glm.hpp"

struct PointLightComponent {
	glm::vec3 position{};
	glm::vec3 ambient{};
	glm::vec3 diffuse{};
	glm::vec3 specular{};
	glm::vec3 attenuation{};
	float intensity{};
	bool castShadow{false};

	PointLightComponent() = default;

	explicit PointLightComponent(
		const glm::vec3 pos,
		const glm::vec3 a,
		const glm::vec3 dif,
		const glm::vec3 s,
		const glm::vec3 att,
		const float i,
		bool cs)
		: position(pos),
		  ambient(a),
		  diffuse(dif),
		  specular(s),
		  attenuation(att),
		  intensity(i),
		  castShadow(cs) {
	}
};
