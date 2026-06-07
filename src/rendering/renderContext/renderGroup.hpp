#pragma once
#include "../material/material.hpp"

struct RenderGroup {
	size_t entityID;
	MaterialBatch matBatch;
};

struct InstanceGroup : RenderGroup {
	std::vector<float> transforms{}; // P,R,S data
};
