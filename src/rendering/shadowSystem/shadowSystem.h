#pragma once
#include <memory>
#include <array>

class EventBus;
struct UpdateShadowMapEvent;
class DirectionalShadow;
class OmnidirectionalShadow;
class PerspectiveShadow;
struct RenderContext;
class UniformBuffer;

class ShadowSystem {
public:
	ShadowSystem();

	~ShadowSystem();

	[[nodiscard]]
	const UniformBuffer* ubo() const;

	[[nodiscard]]
	const std::array<uint32_t, 3>& shadowMaps() const;

	void configure(const RenderContext& ctx, EventBus& eventBus);

private:
	void directionalShadowPass(const RenderContext& ctx) const;

	void omnidirectionalShadowPass(const RenderContext& ctx) const;

	void perspectiveShadowPass(const RenderContext& ctx) const;

	void onGuiUpdate(const UpdateShadowMapEvent& event);

	const RenderContext* mCtx{};
	std::array<uint32_t, 3> mShadowMaps{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::unique_ptr<DirectionalShadow> mDirShadow;
	std::unique_ptr<OmnidirectionalShadow> mOmnidirShadow;
	std::unique_ptr<PerspectiveShadow> mPersShadow;
};
