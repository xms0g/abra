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
		glm::vec3 direction{};
		glm::vec3 position{};
		glm::vec3 ambient{};
		glm::vec3 diffuse{};
		glm::vec3 specular{};
		glm::vec3 attenuation; // (Kc, Kl, kq)
		float cutOff;
		float outerCutOff;
		float intensity{1.0f};
		bool castShadow{false};
	} light;
};
