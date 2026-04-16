#pragma once
#include <vector>
#include <memory>
#include "glm/glm.hpp"

class VertexBuffer;
class VertexArray;
struct Vertex;
class IndexBuffer;

class Mesh {
public:
	Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	~Mesh();

	Mesh(const Mesh&) = delete;

	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other) noexcept;

	Mesh& operator=(Mesh&& other) noexcept;

	[[nodiscard]] const std::vector<Vertex>& vertices() const;

	[[nodiscard]] const std::vector<uint32_t>& indices() const;

	[[nodiscard]] const glm::vec3& min() const;

	[[nodiscard]] const glm::vec3& max() const;

	void bind() const;

	void unbind() const;

	void enableInstanceAttributes(size_t offset) const;

	void uploadToGPU();

private:
	// mesh Data
	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;
	std::unique_ptr<VertexArray> mVAO;
	std::unique_ptr<VertexBuffer> mVBO;
	std::unique_ptr<IndexBuffer> mIBO;
	// Bounding Volume
	glm::vec3 mMin{}, mMax{};
};
