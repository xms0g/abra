#pragma once
#include "../pushConstant.hpp"

struct MaterialPushConstants {
	uint32_t flags;
	float heightScale;
	float alphaCutoff;
	glm::vec3 color;

	static const PushConstantLayout layout;
};

constexpr PushConstantLayout MaterialPushConstants::layout = {
	.constants = {{
		PUSH_CONSTANT_FIELD("material.flags", MaterialPushConstants, flags, PushConstantType::UInt),
		PUSH_CONSTANT_FIELD("material.heightScale", MaterialPushConstants, heightScale, PushConstantType::Float),
		PUSH_CONSTANT_FIELD("material.alphaCutoff", MaterialPushConstants, alphaCutoff, PushConstantType::Float),
		PUSH_CONSTANT_FIELD("material.color", MaterialPushConstants, color, PushConstantType::Vec3),

	}},
	.count = 4
};
