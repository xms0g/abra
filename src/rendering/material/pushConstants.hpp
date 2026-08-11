#pragma once

struct MaterialPushConstants {
	uint32_t flags;
	float heightScale;
	float alphaCutoff;
	glm::vec3 color;
};