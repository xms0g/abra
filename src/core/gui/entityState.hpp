#pragma once
#include <cstdint>
#include "glm/glm.hpp"

struct EntityState {
	size_t id{0};
	uint32_t debugMode{0};
	bool isDirty{false};

	struct {
		glm::vec3 position{0.0f};
		glm::vec3 rotation{0.0f};
		glm::vec3 scale{1.0f};
	} transform;

	struct {
		glm::vec4 direction{};
		glm::vec4 position{};
		glm::vec4 ambient{};
		glm::vec4 diffuse{};
		glm::vec4 specular{};
		glm::vec3 attenuation; // (Kc, Kl, kq)
		glm::vec3 cutOff; // (cutOff, outerCutOff, padding)
		bool castShadow{};
		float intensity{};
	} light;
};
