#pragma once
#include <vector>
#include "entityData.hpp"
#include "../material/material.hpp"

struct InstanceGroup {
	EntityData eData;
	const std::vector<float>* transforms{}; // ptr to P,R,S data
	MaterialBatch matb;
};
