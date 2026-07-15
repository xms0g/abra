#pragma once
#include "../material/material.hpp"

struct RenderGroup {
	size_t entityID;
	MaterialBatch matBatch;
};

struct RenderInstanceGroup : RenderGroup {
	std::vector<float> transforms{}; // P,R,S data
};
