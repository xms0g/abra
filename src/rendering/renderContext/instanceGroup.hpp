#pragma once
#include <vector>
#include "../material/material.hpp"

struct InstanceGroup {
	size_t entityID{};
	std::vector<float> transforms{}; // P,R,S data
	MaterialBatch matBatch;
};
