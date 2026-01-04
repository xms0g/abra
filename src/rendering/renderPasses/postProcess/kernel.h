#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class Kernel final : public IPostEffect {
public:
	explicit Kernel(const std::string& name, const float* kernel, bool enabled = false);

	uint32_t render(uint32_t sceneTexture, uint32_t VAO,
					int& toggle, const std::unique_ptr<FrameBuffer>* renderTargets) const override;

private:
	std::unique_ptr<Shader> shader;
	const float* mKernel;
};
