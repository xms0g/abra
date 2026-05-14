#pragma once
#include "glm/glm.hpp"

struct PointLightComponent {
	glm::vec3 position{};
	glm::vec3 ambient{};
	glm::vec3 diffuse{};
	glm::vec3 specular{};
	float constant{};
	float linear{};
	float quadratic{};
	float intensity{};
	bool castShadow{false};

	PointLightComponent() = default;

	explicit PointLightComponent(
		const glm::vec3 pos,
		const glm::vec3 a,
		const glm::vec3 dif,
		const glm::vec3 s,
		const float kc,
		const float kl,
		const float kq,
		const float i,
		const bool cs)
		: position(pos),
		  ambient(a),
		  diffuse(dif),
		  specular(s),
		  constant(kc),
		  linear(kl),
		  quadratic(kq),
		  intensity(i),
		  castShadow(cs) {
	}
};
