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

void Mesh::enableInstanceAttributes(
	const int32_t stride,
	const size_t offset,
	const size_t modelMatrixOffset,
	const size_t normalMatrixOffset) const {
	// --- Model Matrix (Attribute Locations 7, 8, 9, 10) ---
	// A mat4 takes 4 slots. Each slot is a vec4.
	for (uint32_t i = 0; i < 4; ++i) {
		const uint32_t attribIndex = 7 + i;
		mVAO->setAttribute(attribIndex, 4, GL_FLOAT, false, stride,
		                   reinterpret_cast<void*>(offset + modelMatrixOffset + sizeof(glm::vec4) * i));
		glVertexAttribDivisor(attribIndex, 1);
	}

	// --- Normal Matrix (Attribute Locations 11, 12, 13) ---
	// A mat3 takes 3 slots. Each slot is a vec3.
	for (uint32_t i = 0; i < 3; ++i) {
		const uint32_t attribIndex = 11 + i;
		mVAO->setAttribute(attribIndex, 3, GL_FLOAT, false, stride,
		                   reinterpret_cast<void*>(offset + normalMatrixOffset + sizeof(glm::vec3) * i));
		glVertexAttribDivisor(attribIndex, 1);
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
	// vertex Positions
	mVAO->setAttribute(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
	// vertex normals
	mVAO->setAttribute(1, 3, GL_FLOAT, false, sizeof(Vertex), PTR(normal));
	// vertex texture coords
	mVAO->setAttribute(2, 2, GL_FLOAT, false, sizeof(Vertex), PTR(texcoord));
	// vertex tangent
	mVAO->setAttribute(3, 3, GL_FLOAT, false, sizeof(Vertex), PTR(tangent));
	// vertex bitangent
	mVAO->setAttribute(4, 3, GL_FLOAT, false, sizeof(Vertex), PTR(bitangent));
	// ids
	mVAO->setAttribute(5, 4, GL_INT, false, sizeof(Vertex), PTR(boneIDs));
	// weights
	mVAO->setAttribute(6, 4, GL_FLOAT, false, sizeof(Vertex), PTR(weights));

	mVAO->unbind();
}
