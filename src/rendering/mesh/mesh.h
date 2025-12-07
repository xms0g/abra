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

struct InstanceData {
	glm::mat4 model;
	glm::mat3 normalMatrix;
	float padding[3];
};

class Mesh {
public:
	Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	[[nodiscard]] const std::vector<Vertex>& vertices() const { return mVertices; }

	[[nodiscard]] const std::vector<uint32_t>& indices() const { return mIndices; }

	[[nodiscard]] const glm::vec3& min() const { return mMin; }

	[[nodiscard]] const glm::vec3& max() const { return mMax; }

	[[nodiscard]] bool isVisible() const { return mIsVisible; }

	void setVisible(const bool visible) { mIsVisible = visible; }

	void bind() const;

	void unbind() const;

	void enableInstanceAttributes(uint32_t instanceVBO, size_t offset) const;
private:
	// mesh Data
	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;
	uint32_t mVAO{}, mVBO{}, mEBO{};
	// Bounding Volume
	glm::vec3 mMin{}, mMax{};
	bool mIsVisible{true};
};
