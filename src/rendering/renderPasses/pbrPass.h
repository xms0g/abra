#pragma once
#include <memory>
#include "IRenderPass.hpp"

class CubemapBuffer;
class Shader;

class PBRPass final : public IRenderPass {
public:
	~PBRPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	std::unique_ptr<Shader> mEquirectangularToCube;
	std::unique_ptr<Shader> mIrradianceConv;

	std::unique_ptr<CubemapBuffer> mEnvMapBuffer;
	std::unique_ptr<CubemapBuffer> mIrradianceMapBuffer;
};
