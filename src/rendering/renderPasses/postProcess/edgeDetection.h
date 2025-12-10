#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class EdgeDetection final : public IPostEffect {
public:
	explicit EdgeDetection(const std::string& name, bool enabled = false);

	uint32_t render(uint32_t sceneTexture, uint32_t VAO,
	                int& toggle, const std::unique_ptr<FrameBuffer>* renderTargets) const override;

private:
	std::unique_ptr<Shader> shader;

	static constexpr float kernel[9] = {
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	};
};
