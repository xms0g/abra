#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class Kernel final : public IPostEffect {
public:
	explicit Kernel(const std::string& name, const float* kernel, bool enabled = false);

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		RenderTargetType& renderTargets) const override;

private:
	std::unique_ptr<Shader> shader;
	const float* mKernel;
};
