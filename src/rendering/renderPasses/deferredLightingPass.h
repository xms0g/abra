#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "IRenderPass.hpp"
#include "../models/quad.h"
#include "../models/cube.h"

class CubemapBuffer;
class FrameBuffer;
class Shader;

class DeferredLightingPass final : public IRenderPass {
public:
	~DeferredLightingPass() override;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	std::unique_ptr<Shader> mShader;
	std::unique_ptr<Models::SingleQuad> mQuad;
};
