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

	void configure(const RenderContext& ctx, EventBus& eventBus);

private:
	void directionalShadowPass() const;

	void omnidirectionalShadowPass() const;

	void perspectiveShadowPass() const;

	void onGuiUpdate(const UpdateShadowMapEvent& event);

	const RenderContext* mCtx{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::unique_ptr<DirectionalShadow> mDirShadow;
	std::unique_ptr<OmnidirectionalShadow> mOmnidirShadow;
	std::unique_ptr<PerspectiveShadow> mPersShadow;
};
