#pragma once
#include <memory>
#include "basePostEffect.hpp"

class RenderGraph;
struct RenderContext;
class Shader;
class FrameBuffer;

class Bloom final : public BasePostEffect {
public:
	explicit Bloom(const std::string& name, bool enabled = false);

	~Bloom() override;

	void configure(const RenderGraph& graph) override;

	uint32_t render(uint32_t vao, uint32_t sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	uint32_t brightFilterPass(uint32_t vao, uint32_t sceneTexture, bool& toggle) const;

	uint32_t blurPass(uint32_t vao, uint32_t sceneTexture, bool& toggle) const;

	[[nodiscard]]
	uint32_t combinePass(
		uint32_t vao,
		uint32_t sceneTexture,
		uint32_t blurTexture,
		const bool& toggle) const;

	std::array<FrameBuffer*, 2> mRenderTargets{};

	const Shader* mBrightFilter;
	const Shader* mBlur;
	const Shader* mCombine;
};
