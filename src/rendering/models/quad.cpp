#include "quad.h"
#include "glad/glad.h"
#include "../mesh/vertexArray.h"
#include "../buffers/vertexBuffer.h"
#include "../../rendering/shader.h"

IQuad::IQuad() {
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

	mVBO = std::make_unique<VertexBuffer>(STATIC);
	mVBO->bind();

	mVBO->setData(&vertices[0], sizeof(vertices), 0);

	VertexLayout layout;
	layout.pushVector<glm::vec2>(0);	// Position
	layout.pushVector<glm::vec2>(1);	// TexCoords

	for (const auto& [type, index, size, normalized, offset, divisor]: layout.attributes()) {
		const auto pointer = reinterpret_cast<void*>(offset);
		VertexArray::setAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, layout.stride(), pointer, divisor);
	}

	VertexArray::unbind();
}

IQuad::~IQuad() = default;

uint32_t IQuad::vao() const {
	return mVAO->id();
}

Model::Quad::Quad() {
	mShader = std::make_unique<Shader>( "models/quad.vert", "models/quad.frag");
	mShader->activate();
	mShader->setInt("screenTexture", 0);
}

const Shader& Model::Quad::shader() const {
	return *mShader;
}


