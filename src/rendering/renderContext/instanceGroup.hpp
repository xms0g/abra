#pragma once
#include "../material/material.hpp"

class Entity;

struct InstanceGroup {
	const Entity* entity{};
	const std::vector<float>* transforms{}; // ptr to P,R,S data
	MaterialBatch mbatch;
};
