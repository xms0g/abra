#pragma once
#include <memory>
#include "IPostEffect.hpp"

class FrameBuffer;
class Shader;

class ToneMapping final : public IPostEffect {
public:
	explicit ToneMapping(const std::string& name, bool enabled = false);

	uint32_t render(uint32_t sceneTexture, uint32_t VAO,
	                int& toggle, const std::unique_ptr<FrameBuffer>* renderTargets) const override;

	float& exposure() { return mExposure; }

private:
	float mExposure{1.1f};
	std::unique_ptr<Shader> shader;
};
