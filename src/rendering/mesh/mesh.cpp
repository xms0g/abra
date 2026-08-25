#include "mesh.hpp"
#include "glad/glad.h"
#include "vertexArray.hpp"
#include "vertex.hpp"
#include "vertexLayout.hpp"
#include "../buffers/vertexBuffer.hpp"
#include "../buffers/indexBuffer.hpp"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	: mVertices(std::move(vertices)),
	  mIndices(std::move(indices)) {
	mMin = glm::vec3(FLT_MAX);
	mMax = glm::vec3(-FLT_MAX);

	for (auto& vertex: mVertices) {
		mMin = glm::min(mMin, vertex.position);
		mMax = glm::max(mMax, vertex.position);
	}
}

Mesh::~Mesh() = default;

Mesh::Mesh(Mesh&& other) noexcept = default;
Mesh& Mesh::operator=(Mesh&& other) noexcept = default;

const VertexArray& Mesh::vao() const {
	return *mVAO;
}

const std::vector<Vertex>& Mesh::vertices() const {
	return mVertices;
}

const std::vector<uint32_t>& Mesh::indices() const {
	return mIndices;
}

const glm::vec3& Mesh::min() const {
	return mMin;
}

const glm::vec3& Mesh::max() const {
	return mMax;
}

void Mesh::enableInstanceAttributes(const uint32_t vao, const size_t offset) {
	glBindVertexArray(vao);

	VertexLayout instancingLayout;
	instancingLayout.pushMatrix<glm::mat4>(7); // Model Matrix (slots 7-10)
	instancingLayout.pushMatrix<glm::mat3>(11); // Normal Matrix (slots 11-13)
	instancingLayout.addPadding(sizeof(glm::vec3)); // padding

	for (const auto& [type, index, size, normalized, attrOffset, divisor]: instancingLayout.attributes()) {
		const auto finalPointer = reinterpret_cast<void*>(offset + attrOffset);

		VertexArray::setAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, instancingLayout.stride(),
		                          finalPointer, divisor);
	}
}

void Mesh::uploadToGPU() {
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	// create vao
	mVAO = std::make_unique<VertexArray>();
	mVAO->bind();

	mVBO = std::make_unique<VertexBuffer>(0, BufferUsage::Static);
	mIBO = std::make_unique<IndexBuffer>(BufferUsage::Static);

	mVBO->copyToMemory(mVertices.data(), 0, static_cast<uint32_t>(mVertices.size() * sizeof(Vertex)));
	mIBO->copyToMemory(mIndices.data(), static_cast<uint32_t>(mIndices.size() * sizeof(uint32_t)));

	// set the vertex attribute pointers
	VertexLayout layout;
	layout.pushVector<glm::vec3>(0); // Position
	layout.pushVector<glm::vec3>(1); // Normal
	layout.pushVector<glm::vec2>(2); // TexCoords
	layout.pushVector<glm::vec3>(3); // Tangent
	layout.pushVector<glm::vec3>(4); // Bitangent
	layout.push<int>(5, 4); // Bone IDs (Integers!)
	layout.push<float>(6, 4); // Bone Weights

	for (const auto& [type, index, size, normalized, offset, divisor]: layout.attributes()) {
		const auto pointer = reinterpret_cast<void*>(offset);
		VertexArray::setAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, layout.stride(), pointer,
		                          divisor);
	}

	VertexArray::unbind();
}
