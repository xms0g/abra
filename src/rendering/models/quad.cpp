#include "quad.h"
#include "glad/glad.h"
#include "../mesh/vertexArray.h"
#include "../buffers/vertexBuffer.h"
#include "../../rendering/shader.h"

Model::Quad::Quad() {
	constexpr float vertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	mVAO = std::make_unique<VertexArray>();
	mVAO->bind();

	mVBO = std::make_unique<VertexBuffer>(0, BufferUsage::Static);
	mVBO->bind();

	mVBO->copyToMemory(&vertices[0], 0, sizeof(vertices));

	VertexLayout layout;
	layout.pushVector<glm::vec2>(0);	// Position
	layout.pushVector<glm::vec2>(1);	// TexCoords

	for (const auto& [type, index, size, normalized, offset, divisor]: layout.attributes()) {
		const auto pointer = reinterpret_cast<void*>(offset);
		VertexArray::setAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, layout.stride(), pointer, divisor);
	}

	VertexArray::unbind();
}

Model::Quad::~Quad() = default;

VertexArray& Model::Quad::vao() const {
	return *mVAO;
}


