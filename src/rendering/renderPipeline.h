#pragma once
#include <memory>
#include <vector>
#include "../ECS/system.hpp"

class RenderQueue;
struct RenderData;
class SyncStateSystem;
class EventBus;
class SSAOPass;
class ShadowSystem;
class DeferredLightingPass;
class DeferredGeometryPass;
class IRenderPass;
class FrameBuffer;
class UniformBuffer;
class Camera;
class Registry;
class LightSystem;
class PostProcessPass;
struct RenderContext;
struct SDL_Window;

typedef void* SDL_GLContext;

class RenderPipeline final : public System {
public:
	explicit RenderPipeline(Registry* registry, SDL_Window* window, SDL_GLContext context);

	~RenderPipeline() override;

	void configure(const Camera& camera, EventBus& eventBus);

	void batchEntities();

	void render();

	static void drawGui();

private:
	void refreshCameraData() const;

	void batchEntity(const Entity& entity) const;

	void sortEntities();
	// Systems
	LightSystem* mLightSystem{};
	std::unique_ptr<ShadowSystem> mShadowSystem;
	std::unique_ptr<SyncStateSystem> mSyncStateSystem;
	// Frame Buffers
	std::unique_ptr<FrameBuffer> mSceneBuffer;
	std::unique_ptr<FrameBuffer> mIntermediateBuffer;
	// Uniform Buffers
	std::unique_ptr<UniformBuffer> mCameraUBO;
	// Render Data
	std::unique_ptr<RenderData> mRenderData;
	// Render Queue
	std::unique_ptr<RenderQueue> mRenderQueue;
	// Render context
	std::unique_ptr<RenderContext> mRenderCtx;
	// Render passes
	std::vector<std::unique_ptr<IRenderPass>> mRenderPasses;
};
