#pragma once
#include <array>
#include <string>

enum class PushConstantType: uint32_t {
	Int,
	UInt,
	Float,
	Vec3,
};

struct PushConstant {
	std::string name;
	size_t offset;
	PushConstantType type;
};

struct PushConstantLayout {
	std::array<PushConstant, 16> constants;
	uint32_t count{};
};
