#pragma once
#include <memory>
#include "IRenderPass.hpp"

class CubemapBuffer;
class FrameBuffer;
class Shader;

class PBRPass final : public IRenderPass {
public:
	~PBRPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	std::unique_ptr<Shader> mEquirectangularToCube;
	std::unique_ptr<Shader> mIrradianceConv;

	uint32_t mIrradianceMap{0};
};
