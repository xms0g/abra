#pragma once
#include "entityData.hpp"
#include "../material/material.hpp"

struct RenderGroup {
	EntityData eData;
	MaterialBatch matb;
};
