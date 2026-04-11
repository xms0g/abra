#pragma once
#include <vector>
#include <memory>
#include "vertexArray.h"
#include "../buffers/vertexBuffer.h"

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
	std::shared_ptr<VertexArray> mVAO;
	std::shared_ptr<VertexBuffer> mVBO;
	std::shared_ptr<IndexBuffer> mIBO;
	// Bounding Volume
	glm::vec3 mMin{}, mMax{};
};
