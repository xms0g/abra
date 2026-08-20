#include "mesh.h"
#include "glad/glad.h"
#include "vertexArray.h"
#include "vertex.hpp"
#include "../buffers/vertexBuffer.h"
#include "../buffers/indexBuffer.h"

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

Mesh::Mesh(Mesh&& other) noexcept
	: mVertices(std::move(other.mVertices)),
	  mIndices(std::move(other.mIndices)),
	  mVAO(std::move(other.mVAO)),
	  mVBO(std::move(other.mVBO)),
	  mIBO(std::move(other.mIBO)),
	  mMin(std::exchange(other.mMin, glm::vec3(0))),
	  mMax(std::exchange(other.mMax, glm::vec3(0))) {
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	if (this != &other) {
		mVertices = std::move(other.mVertices);
		mIndices = std::move(other.mIndices);
		mVAO = std::move(other.mVAO);
		mIBO = std::move(other.mIBO);
		mVBO = std::move(other.mVBO);
		mMin = std::exchange(other.mMin, glm::vec3(0));
		mMax = std::exchange(other.mMax, glm::vec3(0));
	}

	return *this;
}

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

	mVBO = std::make_unique<VertexBuffer>(BufferUsage::Static);
	mIBO = std::make_unique<IndexBuffer>(BufferUsage::Static);
	// bind the buffer to be used
	mVBO->bind();
	mVBO->copyToMemory(mVertices.data(), static_cast<uint32_t>(mVertices.size() * sizeof(Vertex)), 0);

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
