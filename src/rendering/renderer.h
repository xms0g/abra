#pragma once
#include <memory>
#include <vector>
#include "renderGraph.h"
#include "renderContext/renderContext.hpp"
#include "buffers/uniformBuffer.h"
#include "renderContext/renderData.hpp"
#include "renderContext/renderQueue.hpp"
#include "../ECS/system.hpp"

class Window;
class SyncStateSystem;
class EventBus;
class ShadowSystem;
class IRenderPass;
class Camera;
class Registry;
class LightSystem;
class PostProcessPass;

class Renderer final : public System {
public:
	explicit Renderer(Registry& registry, const Camera& camera, Window& window);

	~Renderer() override;

	void configure(const Camera& camera, EventBus& eventBus);

	void render();

	static void drawGui();

private:
	void createSystems(Registry& registry);

	void createUniformBuffers(const Camera& camera);

	void createRenderQueues();

	void createRenderPasses(EventBus& eventBus);

	void createFrameBuffers();

	void configureSystems(EventBus& eventBus);

	void configureShaders();

	void refreshCameraData() const;

	void sortEntities();

	RenderGraph mGraph{};
	// Systems
	LightSystem* mLightSystem{};
	std::unique_ptr<ShadowSystem> mShadowSystem;
	std::unique_ptr<SyncStateSystem> mSyncStateSystem;
	// Uniform Buffers
	UniformBuffer mCameraUBO{};
	// Render Data
	RenderData mRenderData{};
	// Render Queue
	QueueRegistry mQueueRegistry{};
	// Render context
	RenderContext mRenderCtx{};
};
