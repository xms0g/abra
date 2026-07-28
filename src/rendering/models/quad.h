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
	VertexArray& vao() const;

private:
	std::unique_ptr<VertexArray> mVAO;
	std::unique_ptr<VertexBuffer> mVBO;
};
}
