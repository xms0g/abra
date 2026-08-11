#pragma once

struct MaterialPushConstant {
	uint32_t flags;
	float heightScale;
	float alphaCutoff;
	glm::vec3 color;
};