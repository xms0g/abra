#pragma once
#include "../material/material.hpp"

class Entity;

struct RenderGroup {
	const Entity* entity{};
	MaterialBatch mbatch;
};
