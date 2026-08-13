#pragma once
#include <array>
#include <string>

#define MAX_PUSH_CONSTANTS 16
#define PUSH_CONSTANT_FIELD(Name, Type, Field, PCType)\
	{.name = Name, .offset = offsetof(Type, Field), .type = PCType}

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
	std::array<PushConstant, MAX_PUSH_CONSTANTS> constants;
	uint32_t count{};
};
