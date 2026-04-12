#pragma once
#include <vector>
#include "entityData.hpp"
#include "../material/material.hpp"

struct InstanceGroup {
	EntityCore entity;
	const std::vector<float>* transforms{}; // ptr to P,R,S data
	MaterialBatch matBatch;
};
