#pragma once
#include <array>
#include <string>

#define MAX_PUSH_CONSTANT_COUNT 16
#define PUSH_CONSTANT(Name, Type, Field, PCType)\
	{.name = Name, .offset = offsetof(Type, Field), .type = PCType}

enum class PushConstantType: uint32_t {
	Int,
	UInt,
	Float,
	Vec3,
	Mat3,
	Mat4
};

struct PushConstant {
	std::string name;
	size_t offset;
	PushConstantType type;
};

struct PushConstantLayout {
	std::array<PushConstant, MAX_PUSH_CONSTANT_COUNT> constants;
	uint32_t count{};
	size_t baseOffset{};
};
