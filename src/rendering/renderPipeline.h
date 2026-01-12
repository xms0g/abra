#pragma once
#include <memory>
#include <vector>
#include "renderContext/renderQueue.hpp"
#include "../ECS/system.hpp"

class SSAOPass;
struct RenderContext;
class Shader;
class ShadowPass;
class DeferredLightingPass;
class DeferredGeometryPass;
class IRenderPass;
class FrameBuffer;
class UniformBuffer;
class Camera;
class Registry;
class LightSystem;
class SkyboxSystem;
class PostProcessPass;

class RenderPipeline final : public System {
public:
	explicit RenderPipeline(Registry* registry);

	~RenderPipeline() override;

	[[nodiscard]] PostProcessPass& postProcess() const;

	void configure(const Camera& camera);

	void batchEntities();

	void render();

private:
	void refreshCameraData() const;

	void batchEntity(const Entity& entity);

	void sortEntities();
	// Systems
	LightSystem* mLightSystem{};
	//Shaders
	std::unique_ptr<Shader> opaque;
	std::unique_ptr<Shader> cutout;
	std::unique_ptr<Shader> blend;
	std::unique_ptr<Shader> unlit;
	std::unique_ptr<Shader> pbr;
	std::unique_ptr<Shader> instancedOpaque;
	std::unique_ptr<Shader> instancedCutout;
	std::unique_ptr<Shader> instancedBlend;
	std::unique_ptr<Shader> skybox;
	// Frame Buffers
	std::unique_ptr<FrameBuffer> mSceneBuffer;
	std::unique_ptr<FrameBuffer> mIntermediateBuffer;
	// Uniform Buffers
	std::unique_ptr<UniformBuffer> mCameraUBO;
	// Render queue
	RenderQueue mRenderQueue;
	// Render context
	std::unique_ptr<RenderContext> mRenderCtx;
	// Render passes
	std::shared_ptr<ShadowPass> mShadowPass;
	std::shared_ptr<PostProcessPass> mPostProcessPass;
	std::shared_ptr<DeferredGeometryPass> mDeferredGeometryPass;
	std::shared_ptr<DeferredLightingPass> mDeferredLightingPass;
	std::shared_ptr<SSAOPass> mSSAOPass;
	std::vector<std::shared_ptr<IRenderPass>> mRenderPasses;
};
