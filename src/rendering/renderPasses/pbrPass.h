#pragma once
#include <memory>
#include "IRenderPass.hpp"

namespace Models {
class SingleQuad;
}

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
	std::unique_ptr<Shader> mPrefilter;
	std::unique_ptr<Shader> mBrdfLUT;

	std::unique_ptr<CubemapBuffer> mEnvMapBuffer;
	std::unique_ptr<CubemapBuffer> mIrradianceMapBuffer;
	std::unique_ptr<CubemapBuffer> mPrefilterMapBuffer;
	std::unique_ptr<FrameBuffer> mBrdfLUTBuffer;

	std::unique_ptr<Models::SingleQuad> mQuad;
};
