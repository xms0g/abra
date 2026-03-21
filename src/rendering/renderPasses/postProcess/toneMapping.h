#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class ToneMapping final : public IPostEffect {
public:
	explicit ToneMapping(const std::string& name, bool enabled = false);

	float& exposure();

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		int& toggle,
		RenderTargetType& renderTargets) const override;

private:
	float mExposure{1.1f};
	std::unique_ptr<Shader> shader;
};
