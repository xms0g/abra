#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
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
class Camera;
class Registry;
class LightSystem;

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

	void createPBRBuffers();

	void createRenderQueues();

	void createRenderPasses(EventBus& eventBus);

	void createFrameBuffers();

	void configureSystems(EventBus& eventBus);

	void refreshCameraData() const;

	void sortEntities();

	uint32_t createEnvMap(Texture& hdrTexture);

	void createIrradianceMap();

	void createPrefilterMap();

	void createBrdfLUT();

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
	// Frame Buffers
	std::unordered_map<std::string, std::unique_ptr<FrameBuffer> > mPBRBuffers;

	static constexpr uint32_t FACES = 6;

	glm::mat4 mCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 mCaptureViews[FACES] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};
};
