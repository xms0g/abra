#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "frameGraph.h"
#include "graphicsEncoder.h"
#include "context/renderContext.hpp"
#include "context/renderData.hpp"
#include "context/renderQueue.hpp"
#include "buffers/uniformBuffer.h"
#include "../ECS/system.hpp"

struct PBRBuffers {
	std::unique_ptr<FrameBuffer> environment;
	std::unique_ptr<FrameBuffer> irradiance;
	std::unique_ptr<FrameBuffer> prefilter;
	std::unique_ptr<FrameBuffer> brdfLUT;
};

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

	void configure(EventBus& eventBus);

	void render();

	static void drawGui();

private:
	void createSystems(Registry& registry);

	void createUniformBuffers();

	void createPBRBuffers();

	void createRenderQueues();

	void createRenderPasses(EventBus& eventBus);

	void createFrameBuffers();

	void configureSystems(EventBus& eventBus);

	void updateUniformBuffers() const;

	void sortEntities();

	TextureView createEnvMap(const MaterialTexture& hdrTexture);

	void createIrradianceMap(TextureView environment);

	void createPrefilterMap(TextureView environment);

	void createBrdfLUT();

	struct alignas(16) UniformBufferObject {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat4 skyView;
		glm::vec4 cameraPos;
		glm::mat4 projection;
	};

	struct ResourceIndexes {
		uint32_t sceneBuffer;
	};

	ResourceIndexes mIndexes{};
	FrameGraph mGraph{};
	// Systems
	LightSystem* mLightSystem{};
	std::unique_ptr<ShadowSystem> mShadowSystem;
	std::unique_ptr<SyncStateSystem> mSyncStateSystem;
	// Uniform Buffers
	UniformBuffer mCameraUBO{};
	// Frame Buffers
	PBRBuffers mPBRBuffers{};
	// Render Data
	RenderData mRenderData{};
	// Render Queue
	QueueRegistry mQueueRegistry{};
	// Render context
	RenderContext mRenderCtx{};
	// Encoder
	GraphicsEncoder mEncoder{};

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
