#pragma once
#include <vector>
#include "../material/material.hpp"

struct InstanceGroup {
	size_t entityID;
	const std::vector<float>* transforms{}; // ptr to P,R,S data
	MaterialBatch matBatch;
};
