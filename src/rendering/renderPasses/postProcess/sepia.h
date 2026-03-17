#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class Sepia final : public IPostEffect {
public:
	explicit Sepia(const std::string& name, bool enabled = false);

	uint32_t render(uint32_t sceneTexture,
	                uint32_t VAO,
	                int& toggle,
	                FrameBuffer** renderTargets) const override;

private:
	std::unique_ptr<Shader> shader;
};
