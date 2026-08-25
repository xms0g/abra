#pragma once
#include "glm/glm.hpp"

struct alignas(16) DirectionalShadowData {
	glm::mat4 lightSpaceMatrix{};
};

struct alignas(16) OmnidirectionalShadowData {
	glm::vec4 omniFarPlane{};
};

struct alignas(16) PerspectiveShadowData {
	glm::mat4 lightSpaceMatrix[4]{};
};

struct alignas(16) ShadowData {
	DirectionalShadowData dirShadowData{};
	OmnidirectionalShadowData omniShadowData{};
	PerspectiveShadowData perShadowData{};
};
