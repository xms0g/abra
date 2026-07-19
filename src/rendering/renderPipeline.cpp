#include "renderPipeline.h"
#include <SDL.h>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "shader.h"
#include "renderCommand.h"
#include "renderBatcher.h"
#include "systems/lightSystem.h"
#include "systems/syncStateSystem.h"
#include "systems/shadowSystem/shadowSystem.h"
#include "buffers/frameBuffer.h"
#include "buffers/uniformBuffer.h"
#include "models/quad.h"
#include "renderContext/renderGroup.hpp"
#include "renderContext/renderableObject.hpp"
#include "renderPasses/IRenderPass.hpp"
#include "renderPasses/deferredGeometryPass.h"
#include "renderPasses/deferredLightingPass.h"
#include "renderPasses/ssaoPass.h"
#include "renderPasses/debugPass.h"
#include "renderPasses/forwardPass.h"
#include "renderPasses/instancedPass.h"
#include "renderPasses/cullingPass.h"
#include "renderPasses/skyboxPass.h"
#include "renderPasses/resolvePass.h"
#include "renderPasses/terrainPass.h"
#include "renderPasses/postProcess/postProcessPass.h"
#include "gui/backend.h"
#include "material/material.hpp"
#include "../config/configManager.h"
#include "../core/camera.h"
#include "../core/window.h"
#include "../ECS/registry.h"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/mesh.hpp"
#include "../event/eventBus.hpp"
#include "../resource/resourceManager.h"

RenderPipeline::RenderPipeline(Registry& registry, const Camera& camera, Window& window) {
	RequireComponent<MeshComponent>();
	RequireComponent<TransformComponent>();
	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		throw std::runtime_error(std::string("ERROR::RENDERER::FAILED_TO_INIT_GLAD"));
	}

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	createRenderQueues();
	createSystems(registry);
	createFrameBuffers();
	createRenderContext(camera);

	GuiBackend::init(&*window, window.glContext(), "#version 410");
}

RenderPipeline::~RenderPipeline() {
	GuiBackend::shutdown();
}

void RenderPipeline::configure(const Camera& camera, EventBus& eventBus) {
	RenderBatcher batcher;
	batcher.build(mRenderData, mRenderQueue, getSystemEntities());

	createRenderPasses(eventBus);
	createUniformBuffers(camera);
	configureSystems(eventBus);
	configureShaders();
}

void RenderPipeline::render() {
	GuiBackend::newFrame();
	refreshCameraData();
	sortEntities();

	mRenderCtx.materialCache.reset();

	mGraph.getResource("sceneBuffer").bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (const auto& pass: mRenderPasses) {
		pass->execute(mRenderCtx, mGraph);
	}
}

void RenderPipeline::drawGui() {
	GuiBackend::renderFrame();
}

void RenderPipeline::createSystems(Registry& registry) {
	mLightSystem = &registry.addSystem<LightSystem>();
	mShadowSystem = std::make_unique<ShadowSystem>();
	mSyncStateSystem = std::make_unique<SyncStateSystem>();
}

void RenderPipeline::createUniformBuffers(const Camera& camera) {
	// Create camera buffer
	mCameraUBO = UniformBuffer{
		DYNAMIC,
		3 * sizeof(glm::mat4) + sizeof(glm::vec4),
		CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding")
	};

	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(camera.zoom()),
		static_cast<float>(CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width")) / static_cast<float>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")),
		camera.znear(), camera.zfar());

	const glm::mat4 invProjectionMat = glm::inverse(projectionMat);

	mCameraUBO.bind();
	mCameraUBO.setData(glm::value_ptr(projectionMat), sizeof(glm::mat4), sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO.setData(glm::value_ptr(invProjectionMat), sizeof(glm::mat4), 2 * sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO.unbind();
}

void RenderPipeline::createRenderQueues() {
	mRenderQueue.set("opaqueInstanced", std::vector<RenderInstanceGroup>());
	mRenderQueue.set("blendInstanced", std::vector<RenderInstanceGroup>());
	mRenderQueue.set("opaque", std::vector<RenderGroup>());
	mRenderQueue.set("blend", std::vector<RenderGroup>());
	mRenderQueue.set("debug", std::vector<RenderGroup>());
	mRenderQueue.set("shadow", std::vector<RenderGroup>());
	mRenderQueue.set("terrain", std::vector<RenderGroup>());
	mRenderQueue.set("skybox", std::vector<RenderGroup>());
	mRenderQueue.set("deferred", std::vector<RenderGroup>());
	mRenderQueue.set("visibleDeferred", std::vector<RenderableObject>());
	mRenderQueue.set("visibleOpaque", std::vector<RenderableObject>());
	mRenderQueue.set("visibleBlend", std::vector<RenderableObject>());
	mRenderQueue.set("visibleDebug", std::vector<RenderableObject>());
}

void RenderPipeline::createRenderPasses(EventBus& eventBus) {
	mRenderPasses.emplace_back(std::make_unique<CullingPass>());

	if (!mRenderQueue.get<std::vector<RenderGroup> >("deferred").empty()) {
		mRenderPasses.emplace_back(std::make_unique<DeferredGeometryPass>());
		mRenderPasses.emplace_back(std::make_unique<SSAOPass>());
		mRenderPasses.emplace_back(std::make_unique<DeferredLightingPass>());
	}

	if (!mRenderQueue.get<std::vector<RenderGroup> >("opaque").empty() ||
	    !mRenderQueue.get<std::vector<RenderGroup> >("blend").empty()) {
		mRenderPasses.emplace_back(std::make_unique<ForwardPass>());
	}

	if (!mRenderQueue.get<std::vector<RenderGroup> >("debug").empty()) {
		mRenderPasses.emplace_back(std::make_unique<DebugPass>());
	}

	if (!mRenderQueue.get<std::vector<RenderInstanceGroup> >("opaqueInstanced").empty() ||
	    !mRenderQueue.get<std::vector<RenderInstanceGroup> >("blendInstanced").empty()) {
		mRenderPasses.emplace_back(std::make_unique<InstancedPass>());
	}

	if (!mRenderQueue.get<std::vector<RenderGroup> >("terrain").empty()) {
		mRenderPasses.emplace_back(std::make_unique<TerrainPass>());
	}

	mRenderPasses.emplace_back(std::make_unique<SkyboxPass>());

	if (CONFIG_MANAGER_INSTANCE.get<bool>("msaa.enabled")) {
		mRenderPasses.emplace_back(std::make_unique<ResolvePass>());
	}

	mRenderPasses.emplace_back(std::make_unique<PostProcessPass>());

	for (const auto& pass: mRenderPasses) {
		pass->configure(mRenderCtx, eventBus);
	}
}

void RenderPipeline::createFrameBuffers() {
	mGraph.addResources(
		"sceneBuffer", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")));

	auto& sceneBuffer = mGraph.getResource("sceneBuffer");

	if (CONFIG_MANAGER_INSTANCE.get<bool>("msaa.enabled")) {
		glEnable(GL_MULTISAMPLE);
		const int32_t sampleCount = CONFIG_MANAGER_INSTANCE.get<int32_t>("msaa.sample_count");

		mGraph.addResources(
			"intermediateBuffer", std::make_unique<FrameBuffer>(
				CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
				CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")));

		auto& intermediateBuffer = mGraph.getResource("intermediateBuffer");
		intermediateBuffer.bind();

		if (CONFIG_MANAGER_INSTANCE.get<bool>("hdr.enabled")) {
			intermediateBuffer.withTextureFP(GL_RGBA)
					.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
					.checkStatus();

			sceneBuffer.bind();
			sceneBuffer.withTextureFPMultisampled(sampleCount, GL_RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, GL_DEPTH_COMPONENT24)
					.checkStatus();
		} else {
			intermediateBuffer.withTexture(GL_RGBA)
					.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
					.checkStatus();

			sceneBuffer.bind();
			sceneBuffer.withTextureMultisampled(sampleCount, GL_RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, GL_DEPTH_COMPONENT24)
					.checkStatus();
		}
	} else {
		sceneBuffer.bind();
		if (CONFIG_MANAGER_INSTANCE.get<bool>("hdr.enabled")) {
			sceneBuffer.withTextureFP(GL_RGBA)
					.withTextureDepth(GL_DEPTH_COMPONENT24, false)
					.checkStatus();
		} else {
			sceneBuffer.withTexture(GL_RGBA)
					.withTextureDepth(GL_DEPTH_COMPONENT24, false)
					.checkStatus();
		}
	}
	sceneBuffer.unbind();

	mGraph.addResources("ssao", std::make_unique<FrameBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"))
	);
	mGraph.getResource("ssao").bind();
	mGraph.getResource("ssao").withTexture(GL_RED).checkStatus();
	mGraph.getResource("ssao").unbind();

	mGraph.addResources("ssaoBlur", std::make_unique<FrameBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"))
	);
	mGraph.getResource("ssaoBlur").bind();
	mGraph.getResource("ssaoBlur").withTexture(GL_RED).checkStatus();
	mGraph.getResource("ssaoBlur").unbind();
	mRenderCtx.ssao.buffer = &mGraph.getResource("ssaoBlur");
}

void RenderPipeline::createRenderContext(const Camera& camera) {
	mRenderCtx.renderData = &mRenderData;
	mRenderCtx.renderQueue = &mRenderQueue;
	mRenderCtx.light.dirLights = &mLightSystem->dirLights();
	mRenderCtx.light.pointLights = &mLightSystem->pointLights();
	mRenderCtx.light.spotLights = &mLightSystem->spotLights();
	mRenderCtx.camera.self = &camera;
}

void RenderPipeline::configureSystems(EventBus& eventBus) const {
	mLightSystem->configure(mRenderCtx, eventBus);
	mSyncStateSystem->configure(mRenderCtx, eventBus);
	mShadowSystem->configure(mRenderCtx, eventBus);
}

void RenderPipeline::configureShaders() {
	const UniformBinding uboBindings[] = {
		{
			CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
			CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"), UniformBuffer::configure
		},
		{
			CONFIG_MANAGER_INSTANCE.get<std::string>("light.block_name"),
			CONFIG_MANAGER_INSTANCE.get<uint32_t>("light.ubo_binding"), UniformBuffer::configure
		},
		{
			CONFIG_MANAGER_INSTANCE.get<std::string>("shadow.block_name"),
			CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"), UniformBuffer::configure
		}
	};

	const TextureBinding shadowMapBindings[] = {
		{"shadowMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot")},
		{"shadowCubemap", CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 1},
		{"persShadowMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 2}
	};

	// Configure shaders
	for (const auto& [name,shader]: RESOURCE_MANAGER_INSTANCE.getShaders()) {
		for (const auto& [blockName, binding, configure]: uboBindings) {
			configure(shader->id(), binding, blockName.c_str());
		}

		RenderCommand::setTextureUnits(shadowMapBindings, *shader);
	}
}

void RenderPipeline::refreshCameraData() {
	mRenderCtx.camera.skyView = glm::mat4(glm::mat3(mRenderCtx.camera.self->viewMatrix()));

	struct alignas(16) PackedView {
		glm::mat4 view;
		glm::vec4 viewPos;
	};

	const PackedView packed = {
		mRenderCtx.camera.self->viewMatrix(),
		glm::vec4(mRenderCtx.camera.self->position(), 1.0)
	};

	mCameraUBO.bind();
	mCameraUBO.setData(&packed, sizeof(PackedView), 0);
}

void RenderPipeline::sortEntities() {
	const glm::vec3& camPos = mRenderCtx.camera.self->position();

	auto sortBatches = [&](auto& batch, bool transparent) -> void {
		std::sort(
			batch.begin(),
			batch.end(),
			[&camPos, &transparent, this](const RenderGroup& a, const RenderGroup& b) {
				const auto aPos = mRenderData.entity.positions[a.entityID];
				const auto bPos = mRenderData.entity.positions[b.entityID];

				const float da = glm::length2(camPos - aPos);
				const float db = glm::length2(camPos - bPos);

				if (transparent)
					return da > db;
				return da < db;
			}
		);
	};

	// Sort opaque objects front to back
	sortBatches(mRenderQueue.get<std::vector<RenderGroup> >("deferred"), false);
	sortBatches(mRenderQueue.get<std::vector<RenderGroup> >("opaque"), false);
	sortBatches(mRenderQueue.get<std::vector<RenderGroup> >("blend"), true);

	// for (auto& [entity, transforms, materials]: renderQueues.blendInstancedGroup) {
	// 	auto transform = *transforms;
	//
	// 	for (int i = 0; i < transform.size(); i += 9) {
	// 		glm::vec3 pos{transform[i], transform[i + 1], transform[i + 2]};
	// 		std::sort(
	// 			positions->begin(),
	// 			positions->end(),
	// 			[&camPos](const glm::vec3& a, const glm::vec3& b) {
	// 				const float da = glm::length2(camPos - a);
	// 				const float db = glm::length2(camPos - b);
	// 				return da > db; // back to front
	// 			}
	// 		);
	// 	}
	// }
}
