#pragma once
#include <memory>
#include <vector>
#include "frameGraph.h"
#include "context/renderContext.hpp"
#include "context/renderData.hpp"
#include "context/renderQueue.hpp"
#include "buffers/uniformBuffer.h"
#include "../ECS/system.hpp"

class Window;
class SyncStateSystem;
class EventBus;
class ShadowSystem;
class IPass;
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

	void refreshCameraData() const;

	void sortEntities();

	FrameGraph mGraph{};
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
