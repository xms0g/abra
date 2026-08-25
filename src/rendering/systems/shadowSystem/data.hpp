#pragma once
#include "glm/glm.hpp"

struct alignas(16) DirectionalShadowData {
	glm::mat4 lightSpaceMatrix{};
};

struct alignas(16) OmnidirectionalShadowData {
	glm::mat4 shadowMatrices[6];
	glm::vec4 posFarPlane;
};

struct alignas(16) PerspectiveShadowData {
	glm::mat4 lightSpaceMatrix[4]{};
};

struct alignas(16) ShadowData {
	DirectionalShadowData dirShadowData{};
	OmnidirectionalShadowData omniShadowData{};
	PerspectiveShadowData perShadowData{};
};
