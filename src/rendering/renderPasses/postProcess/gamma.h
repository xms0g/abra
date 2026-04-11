#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class Gamma final : public IPostEffect {
public:
	explicit Gamma(const std::string& name, bool enabled = false);

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		PingPongBuffer& pingPong) const override;

private:
	std::unique_ptr<Shader> shader;
};
