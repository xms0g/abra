#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
	int boneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float weights[MAX_BONE_INFLUENCE];
};

class Mesh {
public:
	Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	[[nodiscard]] const std::vector<Vertex>& vertices() const;

	[[nodiscard]] const std::vector<uint32_t>& indices() const;

	[[nodiscard]] const glm::vec3& min() const;

	[[nodiscard]] const glm::vec3& max() const;

	void bind() const;

	void unbind() const;

	void enableInstanceAttributes(uint32_t instanceVBO, size_t offset) const;

	void uploadToGPU();
private:
	// mesh Data
	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;
	uint32_t mVAO{}, mVBO{}, mEBO{};
	// Bounding Volume
	glm::vec3 mMin{}, mMax{};
};
