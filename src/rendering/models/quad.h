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

class Quad final : public IQuad {
public:
	Quad();

	~Quad() override = default;

	[[nodiscard]]
	const Shader& shader() const;

private:
	std::unique_ptr<Shader> mShader;
};
}
