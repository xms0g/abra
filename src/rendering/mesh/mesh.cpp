#include "mesh.h"
#include "glad/glad.h"
#include "../instanceBufferBuilder.hpp"

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
	mMin = std::move(other.mMin);
	mMax = std::move(other.mMax);
	mVAO = std::move(other.mVAO);
	mIBO = std::move(other.mIBO);
	mVBO = std::move(other.mVBO);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	if (this != &other) {
		mVertices = std::move(other.mVertices);
		mIndices = std::move(other.mIndices);
		mMin = std::move(other.mMin);
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

void Mesh::enableInstanceAttributes(const uint32_t instanceVBO, const size_t offset) const {
	mVAO->bind();
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	// Model matrix attributes (7–10)
	for (uint32_t i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(7 + i);
		glVertexAttribPointer(7 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
		                      reinterpret_cast<void*>(offset + sizeof(glm::vec4) * i));
		glVertexAttribDivisor(7 + i, 1);
	}

	// Normal matrix attributes (11–13)
	for (uint32_t i = 0; i < 3; ++i) {
		glEnableVertexAttribArray(11 + i);
		glVertexAttribPointer(11 + i, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
		                      reinterpret_cast<void*>(offset + sizeof(glm::mat4) + sizeof(glm::vec3) * i));
		glVertexAttribDivisor(11 + i, 1);
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
	mVBO->setData(mVertices.data(), static_cast<uint32_t>(mVertices.size() * sizeof(Vertex)));

	mIBO->bind();
	mIBO->setData(mIndices.data(), static_cast<uint32_t>(mIndices.size() * sizeof(uint32_t)));

	// set the vertex attribute pointers
	// vertex Positions
	mVAO->setAttribute(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
	// vertex normals
	mVAO->setAttribute(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	// vertex texture coords
	mVAO->setAttribute(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texcoord)));
	// vertex tangent
	mVAO->setAttribute(3, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tangent)));
	// vertex bitangent
	mVAO->setAttribute(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, bitangent)));
	// ids
	mVAO->setAttribute(5, 4, GL_INT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, boneIDs)));
	// weights
	mVAO->setAttribute(6, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, weights)));

	mVAO->unbind();
}
