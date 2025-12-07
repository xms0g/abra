#pragma once
#include <memory>
#include "IRenderPass.hpp"
#include "../models/quad.h"

class FrameBuffer;
class Shader;

class DeferredLightingPass final : public IRenderPass {
public:
	~DeferredLightingPass() override;

	void configureInput(const FrameBuffer* gBuffer);

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	const FrameBuffer* mGBuffer = nullptr;
	std::unique_ptr<Shader> mShader;
	std::unique_ptr<Models::SingleQuad> mQuad;
};
