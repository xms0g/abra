#pragma once
#include <memory>

class VertexArray;
class VertexBuffer;
class Shader;

namespace Model {
class Quad {
public:
	Quad();

	~Quad();

	[[nodiscard]]
	uint32_t vao() const;

private:
	std::unique_ptr<VertexArray> mVAO;
	std::unique_ptr<VertexBuffer> mVBO;
};
}
