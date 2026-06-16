#pragma once
#include <memory>

class Shader;

class IQuad {
public:
	IQuad();

	virtual ~IQuad();

	[[nodiscard]]
	uint32_t vao() const { return mVAO; }

protected:
	uint32_t mVAO{0}, mVBO{0};
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
