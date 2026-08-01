#pragma once
#include <span>
#include "../material/material.hpp"

struct RenderGroup {
	size_t entityID;
	MaterialBatch matBatch;
};

struct RenderInstanceGroup : RenderGroup {
	std::span<const float> transforms{}; // P,R,S data
};
