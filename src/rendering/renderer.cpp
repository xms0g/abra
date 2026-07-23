#include "renderer.h"
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

Renderer::Renderer(Registry& registry, const Camera& camera, Window& window) {
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

	mRenderCtx.renderData = &mRenderData;
	mRenderCtx.queueRegistry = &mQueueRegistry;
	mRenderCtx.camera.camera = &camera;

	GuiBackend::init(&*window, window.glContext(), "#version 410");
}

Renderer::~Renderer() {
	GuiBackend::shutdown();
}

void Renderer::configure(const Camera& camera, EventBus& eventBus) {
	RenderBatcher batcher;
	batcher.build(mRenderData, mQueueRegistry, getSystemEntities());

	createRenderPasses(eventBus);
	createUniformBuffers(camera);
	configureSystems(eventBus);
	configureShaders();
}

void Renderer::render() {
	GuiBackend::newFrame();
	refreshCameraData();
	sortEntities();

	mRenderCtx.materialCache.reset();

	mGraph.getResource("sceneBuffer").bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mGraph.execute(mRenderCtx);
}

void Renderer::drawGui() {
	GuiBackend::renderFrame();
}

void Renderer::createSystems(Registry& registry) {
	mLightSystem = &registry.addSystem<LightSystem>();
	mShadowSystem = std::make_unique<ShadowSystem>();
	mSyncStateSystem = std::make_unique<SyncStateSystem>();
}

void Renderer::createUniformBuffers(const Camera& camera) {
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

void Renderer::createRenderQueues() {
	mQueueRegistry.set<RenderInstanceGroup>("opaqueInstanced");
	mQueueRegistry.set<RenderInstanceGroup>("blendInstanced");
	mQueueRegistry.set<RenderGroup>("opaque");
	mQueueRegistry.set<RenderGroup>("blend");
	mQueueRegistry.set<RenderGroup>("debug");
	mQueueRegistry.set<RenderGroup>("shadow");
	mQueueRegistry.set<RenderGroup>("terrain");
	mQueueRegistry.set<RenderGroup>("skybox");
	mQueueRegistry.set<RenderGroup>("deferred");
	mQueueRegistry.set<RenderableObject>("visibleDeferred");
	mQueueRegistry.set<RenderableObject>("visibleOpaque");
	mQueueRegistry.set<RenderableObject>("visibleBlend");
	mQueueRegistry.set<RenderableObject>("visibleDebug");
}

void Renderer::createRenderPasses(EventBus& eventBus) {
	mGraph.addPass({
		.name = "DeferredLightingPass",
		.isActive = !mQueueRegistry.empty("deferred"),
		.pass = std::make_unique<DeferredLightingPass>()
	});
	mGraph.addPass({
		.name = "DeferredGeometryPass",
		.isActive = !mQueueRegistry.empty("deferred"),
		.pass = std::make_unique<DeferredGeometryPass>()
	});
	mGraph.addPass({
		.name = "ForwardPass",
		.isActive = !mQueueRegistry.empty("opaque")|| !mQueueRegistry.empty("blend"),
		.pass = std::make_unique<ForwardPass>()
	});
	mGraph.addPass({
		.name = "SSAOPass",
		.isActive = !mQueueRegistry.empty("deferred"),
		.pass = std::make_unique<SSAOPass>()
	});
	mGraph.addPass({
		.name = "InstancedPass",
		.isActive = !mQueueRegistry.empty("opaqueInstanced") || !mQueueRegistry.empty("blendInstanced"),
		.pass = std::make_unique<InstancedPass>()
	});
	mGraph.addPass({
		.name = "DebugPass",
		.isActive = !mQueueRegistry.empty("debug"),
		.pass = std::make_unique<DebugPass>()
	});
	mGraph.addPass({
		.name = "TerrainPass",
		.isActive = !mQueueRegistry.empty("terrain"),
		.pass = std::make_unique<TerrainPass>()
	});
	mGraph.addPass({
		.name = "ResolvePass",
		.isActive = CONFIG_MANAGER_INSTANCE.get<bool>("msaa.enabled"),
		.pass = std::make_unique<ResolvePass>()
	});
	mGraph.addPass({.name = "SkyboxPass", .pass = std::make_unique<SkyboxPass>()});
	mGraph.addPass({.name = "CullingPass", .pass = std::make_unique<CullingPass>()});
	mGraph.addPass({.name = "PostProcessPass", .pass = std::make_unique<PostProcessPass>()});
	mGraph.compile();
	mGraph.configure(mRenderCtx, eventBus);
}

void Renderer::createFrameBuffers() {
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

	// GBuffer
	mGraph.addResources(
		"gBuffer", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")));

	auto& gBuffer = mGraph.getResource("gBuffer");
	gBuffer.bind();
	gBuffer.withTextureFP(GL_RGBA) // position
			.withTextureFP(GL_RGBA); // normal
	if (CONFIG_MANAGER_INSTANCE.get<bool>("hdr.enabled")) {
		// albedo
		gBuffer.withTextureFP(GL_RGBA);
	} else {
		gBuffer.withTexture(GL_RGBA);
	}
	// Emissive placed into alpha channels in position, normal, albedo

	gBuffer.withTexture(GL_RGBA) // orm
			.configureAttachments()
			.withTextureDepth(GL_DEPTH_COMPONENT24, false)
			.checkStatus();

	// SSAO
	mGraph.addResources(
		"ssao", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"))
	);
	mGraph.getResource("ssao").bind();
	mGraph.getResource("ssao").withTexture(GL_RED).checkStatus();
	mGraph.getResource("ssao").unbind();

	mGraph.addResources(
		"ssaoBlur", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"))
	);
	mGraph.getResource("ssaoBlur").bind();
	mGraph.getResource("ssaoBlur").withTexture(GL_RED).checkStatus();
	mGraph.getResource("ssaoBlur").unbind();

	// Shadow Maps
	mGraph.addResources(
		"directional", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height")));
	mGraph.getResource("directional").bind();
	mGraph.getResource("directional").withTextureDepth(GL_DEPTH_COMPONENT24, true).checkStatus();
	mGraph.getResource("directional").unbind();

	mGraph.addResources(
		"point", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height")));
	mGraph.getResource("point").bind();
	mGraph.getResource("point").withTextureCubemapDepthArray(
				CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_point"), GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mGraph.getResource("point").unbind();

	mGraph.addResources(
		"spot", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height")));
	mGraph.getResource("spot").bind();
	mGraph.getResource("spot").withTextureDepthArray(
				CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_spot"), GL_DEPTH_COMPONENT24, true)
			.checkStatus();

	// PostProcess Render Targets
	auto addPPRenderTarget = [&](const std::string& name) {
		mGraph.addResources(
			name, std::make_unique<FrameBuffer>(
				CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
				CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")));

		auto& target = mGraph.getResource(name);
		if (CONFIG_MANAGER_INSTANCE.get<bool>("hdr.enabled")) {
			target.withTextureFP(GL_RGBA);
		} else {
			target.withTexture(GL_RGBA);
		}
		target.checkStatus();
	};

	addPPRenderTarget("ping");
	addPPRenderTarget("pong");
	addPPRenderTarget("bloomPing");
	addPPRenderTarget("bloomPong");
}

void Renderer::configureSystems(EventBus& eventBus) {
	mLightSystem->configure(mRenderCtx, eventBus);
	mSyncStateSystem->configure(mRenderCtx, eventBus);
	mShadowSystem->configure(mRenderCtx, mGraph, eventBus);
}

void Renderer::configureShaders() {
	const UniformBinding uboBindings[] = {
		{
			.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
			.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			.configure = UniformBuffer::configure
		},
		{
			.name = CONFIG_MANAGER_INSTANCE.get<std::string>("light.block_name"),
			.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("light.ubo_binding"),
			.configure = UniformBuffer::configure
		},
		{
			.name = CONFIG_MANAGER_INSTANCE.get<std::string>("shadow.block_name"),
			.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"),
			.configure = UniformBuffer::configure
		}
	};

	const TextureBinding shadowMapBindings[] = {
		{.name = "shadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot")},
		{.name = "shadowCubemap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 1},
		{.name = "persShadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 2}
	};

	// Configure shaders
	for (const auto& [name,shader]: RESOURCE_MANAGER_INSTANCE.getShaders()) {
		for (const auto& [blockName, binding, configure]: uboBindings) {
			configure(shader->id(), binding, blockName.c_str());
		}

		RenderCommand::setTextureUnits(shadowMapBindings, *shader);
	}
}

void Renderer::refreshCameraData() {
	mRenderCtx.camera.skyView = glm::mat4(glm::mat3(mRenderCtx.camera.camera->viewMatrix()));

	struct alignas(16) PackedView {
		glm::mat4 view;
		glm::vec4 viewPos;
	};

	const PackedView packed = {
		.view = mRenderCtx.camera.camera->viewMatrix(),
		.viewPos = glm::vec4(mRenderCtx.camera.camera->position(), 1.0)
	};

	mCameraUBO.bind();
	mCameraUBO.setData(&packed, sizeof(PackedView), 0);
}

void Renderer::sortEntities() {
	const glm::vec3& camPos = mRenderCtx.camera.camera->position();

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
	sortBatches(mQueueRegistry.get<RenderGroup>("deferred"), false);
	sortBatches(mQueueRegistry.get<RenderGroup>("opaque"), false);
	sortBatches(mQueueRegistry.get<RenderGroup>("blend"), true);

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
