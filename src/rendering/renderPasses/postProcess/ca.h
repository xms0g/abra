#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class CA final : public IPostEffect {
public:
	explicit CA(const std::string& name, bool enabled = false);

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		int& toggle,
		FrameBuffer** renderTargets) const override;

	float& intensity() { return mIntensity; }

private:
	float mIntensity{};
	std::unique_ptr<Shader> shader;
};
