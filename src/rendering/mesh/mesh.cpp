#include "mesh.h"
#include "glad/glad.h"

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

Mesh::Mesh(Mesh&& other) noexcept {
	mVertices = std::move(other.mVertices);
	mIndices = std::move(other.mIndices);
	mMin = other.mMin;
	mMax = other.mMax;
	mVAO = std::move(other.mVAO);
	mIBO = std::move(other.mIBO);
	mVBO = std::move(other.mVBO);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	if (this != &other) {
		mVertices = std::move(other.mVertices);
		mIndices = std::move(other.mIndices);
		mMin = other.mMin;
		mMax = other.mMax;
		mVAO = std::move(other.mVAO);
		mIBO = std::move(other.mIBO);
		mVBO = std::move(other.mVBO);
	}
	return *this;
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

void Mesh::bind() const {
	mVAO->bind();
}

void Mesh::unbind() const {
	mVAO->unbind();
}

void Mesh::enableInstanceAttributes(const int32_t stride, const size_t offset) const {
	VertexLayout instancingLayout;

	instancingLayout.pushMatrix<glm::mat4>(7, 1); // Model Matrix (slots 7-10)
	instancingLayout.pushMatrix<glm::mat3>(11, 1); // Normal Matrix (slots 11-13)

	for (const auto& [type, index, size, normalized, attrOffset, divisor]: instancingLayout.getAttributes()) {
		const auto finalPointer = reinterpret_cast<void*>(offset + attrOffset);
		mVAO->setAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, stride, finalPointer);
		glVertexAttribDivisor(index, divisor);
	}
}

void Mesh::uploadToGPU() {
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	// create vao
	mVAO = std::make_unique<VertexArray>();
	mVAO->bind();

	mVBO = std::make_unique<VertexBuffer>(STATIC);
	mIBO = std::make_unique<IndexBuffer>(STATIC);
	// bind the buffer to be used
	mVBO->bind();
	mVBO->setData(mVertices.data(), static_cast<uint32_t>(mVertices.size() * sizeof(Vertex)), 0);

	mIBO->bind();
	mIBO->setData(mIndices.data(), static_cast<uint32_t>(mIndices.size() * sizeof(uint32_t)));

	// set the vertex attribute pointers
	VertexLayout layout;
	layout.push<float>(0, 3); // Position
	layout.push<float>(1, 3); // Normal
	layout.push<float>(2, 2); // TexCoords
	layout.push<float>(3, 3); // Tangent
	layout.push<float>(4, 3); // Bitangent
	layout.push<int>(5, 4);	// Bone IDs (Integers!)
	layout.push<float>(6, 4);	// Bone Weights

	for (const auto& [type, index, size, normalized, offset, divisor]: layout.getAttributes()) {
		const auto pointer = reinterpret_cast<void*>(offset);
		mVAO->setAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, layout.getStride(), pointer);
	}

	mVAO->unbind();
}
