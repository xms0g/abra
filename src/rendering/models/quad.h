#pragma once
#include <memory>

class VertexArray;
class VertexBuffer;
class Shader;

class IQuad {
public:
	IQuad();

	virtual ~IQuad();

	[[nodiscard]]
	uint32_t vao() const;

protected:
	std::unique_ptr<VertexArray> mVAO;
	std::unique_ptr<VertexBuffer> mVBO;
};

namespace Model {
class SingleQuad final : public IQuad {
public:
	SingleQuad() = default;

	~SingleQuad() override = default;
};
}
