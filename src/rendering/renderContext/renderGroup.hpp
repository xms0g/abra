#pragma once
#include "entityData.hpp"
#include "../material/material.hpp"

struct RenderGroup {
	EntityCore entity;
	MaterialBatch matBatch;
};
