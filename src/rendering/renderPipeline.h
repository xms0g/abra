#pragma once
#include <memory>
#include <vector>
#include "renderContext/renderQueue.hpp"
#include "../ECS/system.hpp"

class EventBus;
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
class SDL_Window;

typedef void* SDL_GLContext;

class RenderPipeline final : public System {
public:
	explicit RenderPipeline(Registry* registry, SDL_Window* window, SDL_GLContext context);

	~RenderPipeline() override;

	void configure(const Camera& camera, EventBus& eventBus);

	void batchEntities();

	void render();

	void drawGui();

private:
	void refreshCameraData() const;

	void batchEntity(const Entity& entity);

	void sortEntities();
	// Systems
	LightSystem* mLightSystem{};
	//Shaders
	std::vector<std::unique_ptr<Shader>> mShaders;
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
	std::shared_ptr<DeferredGeometryPass> mDeferredGeometryPass;
	std::shared_ptr<DeferredLightingPass> mDeferredLightingPass;
	std::shared_ptr<SSAOPass> mSSAOPass;
	std::vector<std::shared_ptr<IRenderPass>> mRenderPasses;
};
