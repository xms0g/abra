#pragma once
#include "glm/glm.hpp"
#include "../pushConstant.hpp"

struct TransformPushConstants {
	glm::mat4 model;
	glm::mat3 normal;

	static const PushConstantLayout layout;
};

constexpr PushConstantLayout TransformPushConstants::layout = {
	.constants = {{
		PUSH_CONSTANT("model", TransformPushConstants, model, PushConstantType::Mat4),
		PUSH_CONSTANT("normalMatrix", TransformPushConstants, normal, PushConstantType::Mat3),
	}},
	.count = 2
};