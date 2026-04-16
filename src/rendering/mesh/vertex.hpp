#pragma once
#include "glm/glm.hpp"

struct Vertex {
#define MAX_BONE_INFLUENCE 4
	// position
	glm::vec3 position;
	// normal
	glm::vec3 normal;
	// texCoords
	glm::vec2 texcoord;
	// tangent
	glm::vec3 tangent;
	// bitangent
	glm::vec3 bitangent;
	//bone indexes which will influence this vertex
	int32_t boneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float weights[MAX_BONE_INFLUENCE];
};
