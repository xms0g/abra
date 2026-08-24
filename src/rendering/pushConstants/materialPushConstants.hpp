#pragma once
#include "../pushConstant.hpp"
#include "../material/material.hpp"

struct MaterialPushConstants {
	MaterialFlag flags;
	float heightScale;
	float alphaCutoff;
	glm::vec3 color;

	static const PushConstantLayout layout;
};

constexpr PushConstantLayout MaterialPushConstants::layout = {
	.constants = {{
		PUSH_CONSTANT("material.flags", MaterialPushConstants, flags, PushConstantType::UInt),
		PUSH_CONSTANT("material.heightScale", MaterialPushConstants, heightScale, PushConstantType::Float),
		PUSH_CONSTANT("material.alphaCutoff", MaterialPushConstants, alphaCutoff, PushConstantType::Float),
		PUSH_CONSTANT("material.color", MaterialPushConstants, color, PushConstantType::Vec3),

	}},
	.count = 4
};
