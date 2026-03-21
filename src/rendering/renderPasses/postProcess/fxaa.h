#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class FXAA final : public IPostEffect {
public:
	explicit FXAA(const std::string& name, bool enabled = false);

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		int& toggle,
		FrameBuffer** renderTargets) const override;

private:
	std::unique_ptr<Shader> shader;
};
