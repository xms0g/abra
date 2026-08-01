#pragma once
#include <span>

struct InstanceComponent {
	std::span<const float> transforms{};

	InstanceComponent() = default;
	explicit InstanceComponent(const std::span<const float> t) : transforms(t) {}
};
