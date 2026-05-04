#pragma once
#include <memory>
#include <array>
#include "../IRenderPass.hpp"

class DirectionalShadowPass;
class OmnidirectionalShadowPass;
class PerspectiveShadowPass;
struct RenderContext;
class UniformBuffer;

class ShadowPass final: public IRenderPass {
public:
	ShadowPass();

	~ShadowPass() override;

	[[nodiscard]]
	const UniformBuffer* ubo() const;

	[[nodiscard]]
	const std::array<uint32_t, 3>& shadowMaps() const;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	void directionalShadowPass(const RenderContext& ctx) const;

	void omnidirectionalShadowPass(const RenderContext& ctx) const;

	void perspectiveShadowPass(const RenderContext& ctx) const;

	std::array<uint32_t, 3> mShadowMaps{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::unique_ptr<DirectionalShadowPass> dirShadowPass;
	std::unique_ptr<OmnidirectionalShadowPass> omnidirShadowPass;
	std::unique_ptr<PerspectiveShadowPass> persShadowPass;
};
