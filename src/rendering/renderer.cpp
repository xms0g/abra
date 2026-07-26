#include "renderer.h"
#include <SDL.h>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "shader.h"
#include "renderCommand.h"
#include "command.hpp"
#include "batcher.h"
#include "systems/lightSystem.h"
#include "systems/syncStateSystem.h"
#include "systems/shadowSystem/shadowSystem.h"
#include "buffers/frameBuffer.h"
#include "buffers/uniformBuffer.h"
#include "models/quad.h"
#include "context/renderGroup.hpp"
#include "context/visibleObject.hpp"
#include "passes/deferredGeometry.h"
#include "passes/deferredLighting.h"
#include "passes/ssao.h"
#include "passes/debug.h"
#include "passes/forwardOpaque.h"
#include "passes/instancedOpaque.h"
#include "passes/culling.h"
#include "passes/skybox.h"
#include "passes/resolve.h"
#include "passes/terrain.h"
#include "passes/postProcess/postProcess.h"
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
#include "passes/forwardUnlit.h"

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
	mRenderCtx.camera = &camera;

	GuiBackend::init(&*window, window.glContext(), "#version 410");
}

Renderer::~Renderer() {
	GuiBackend::shutdown();
}

void Renderer::configure(const Camera& camera, EventBus& eventBus) {
	Batcher batcher;
	batcher.build(mRenderData, mQueueRegistry, getSystemEntities());

	createRenderPasses(eventBus);
	createUniformBuffers(camera);
	configureSystems(eventBus);
}

void Renderer::render() {
	GuiBackend::newFrame();
	refreshCameraData();
	sortEntities();

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
		4 * sizeof(glm::mat4) + sizeof(glm::vec4),
		CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding")
	};

	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(camera.zoom()),
		static_cast<float>(CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width")) / static_cast<float>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")),
		camera.znear(), camera.zfar());

	const glm::mat4 invProjectionMat = glm::inverse(projectionMat);

	mCameraUBO.bind();
	mCameraUBO.setData(glm::value_ptr(projectionMat), sizeof(glm::mat4), 2 * sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO.setData(glm::value_ptr(invProjectionMat), sizeof(glm::mat4), 3 * sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO.unbind();
}

void Renderer::createRenderQueues() {
	mQueueRegistry.set<RenderInstanceGroup>("opaqueInstanced");
	mQueueRegistry.set<RenderInstanceGroup>("blendInstanced");
	mQueueRegistry.set<RenderGroup>("opaque");
	mQueueRegistry.set<RenderGroup>("unlit");
	mQueueRegistry.set<RenderGroup>("blend");
	mQueueRegistry.set<RenderGroup>("debug");
	mQueueRegistry.set<RenderGroup>("shadow");
	mQueueRegistry.set<RenderGroup>("terrain");
	mQueueRegistry.set<RenderGroup>("skybox");
	mQueueRegistry.set<RenderGroup>("deferred");
	mQueueRegistry.set<DrawCommand>("DeferredCommands");
	mQueueRegistry.set<DrawCommand>("OpaqueCommands");
	mQueueRegistry.set<DrawCommand>("UnlitCommands");
	mQueueRegistry.set<DrawCommand>("BlendCommands");
	mQueueRegistry.set<DrawCommand>("DebugCommands");
}

void Renderer::createRenderPasses(EventBus& eventBus) {
	mGraph.addPass(
		"DeferredLightingPass",
		!mQueueRegistry.empty("deferred"),
		std::make_unique<DeferredLightingPass>(),
		{"gBuffer", "ssaoBlur"},
		{"sceneBuffer"}
	);
	mGraph.addPass(
		"DeferredGeometryPass",
		!mQueueRegistry.empty("deferred"),
		std::make_unique<DeferredGeometryPass>(),
		{"DeferredCommands"},
		{"gBuffer"}
	);
	mGraph.addPass(
		"ForwardOpaquePass",
		!mQueueRegistry.empty("opaque"),
		std::make_unique<ForwardOpaquePass>(),
		{"sceneBuffer", "OpaqueCommands"},
		{"sceneBuffer"}
	);
	mGraph.addPass(
		"ForwardUnlitPass",
		!mQueueRegistry.empty("unlit"),
		std::make_unique<ForwardUnlitPass>(),
		{"sceneBuffer", "UnlitCommands"},
		{"sceneBuffer"}
	);
	mGraph.addPass(
		"SSAOPass",
		!mQueueRegistry.empty("deferred"),
		std::make_unique<SSAOPass>(),
		{"gBuffer"},
		{"ssao", "ssaoBlur"}
	);
	mGraph.addPass(
		"InstancedPass",
		!mQueueRegistry.empty("opaqueInstanced") || !mQueueRegistry.empty("blendInstanced"),
		std::make_unique<InstancedOpaquePass>(),
		{"sceneBuffer"},
		{"sceneBuffer"}
	);
	mGraph.addPass(
		"DebugPass",
		!mQueueRegistry.empty("debug"),
		std::make_unique<DebugPass>(),
		{"sceneBuffer", "DebugCommands"},
		{"sceneBuffer"}
	);
	mGraph.addPass(
		"TerrainPass",
		!mQueueRegistry.empty("terrain"),
		std::make_unique<TerrainPass>(),
		{"sceneBuffer"},
		{"sceneBuffer"}
	);
	mGraph.addPass(
		"ResolvePass",
		CONFIG_MANAGER_INSTANCE.get<bool>("msaa.enabled"),
		std::make_unique<ResolvePass>(),
		{"sceneBuffer"},
		{"intermediateBuffer"}
	);
	mGraph.addPass(
		"SkyboxPass",
		true,
		std::make_unique<SkyboxPass>(),
		{"sceneBuffer"},
		{"sceneBuffer"});
	mGraph.addPass(
		"CullingPass",
		true,
		std::make_unique<CullingPass>(),
		{},
		{"OpaqueCommands", "UnlitCommands", "BlendCommands", "DeferredCommands", "DebugCommands"});
	mGraph.addPass(
		"PostProcessPass",
		true,
		std::make_unique<PostProcessPass>(),
		{"sceneBuffer"},
		{"frameBuffer"});
	mGraph.compile();
	mGraph.configure(mRenderCtx, eventBus);
}

void Renderer::createFrameBuffers() {
	mGraph.addResource(
		"sceneBuffer", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height")));

	auto& sceneBuffer = mGraph.getResource("sceneBuffer");

	if (CONFIG_MANAGER_INSTANCE.get<bool>("msaa.enabled")) {
		glEnable(GL_MULTISAMPLE);
		const int32_t sampleCount = CONFIG_MANAGER_INSTANCE.get<int32_t>("msaa.sample_count");

		mGraph.addResource(
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
	mGraph.addResource(
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
	mGraph.addResource(
		"ssao", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"))
	);
	mGraph.getResource("ssao").bind();
	mGraph.getResource("ssao").withTexture(GL_RED).checkStatus();
	mGraph.getResource("ssao").unbind();

	mGraph.addResource(
		"ssaoBlur", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"))
	);
	mGraph.getResource("ssaoBlur").bind();
	mGraph.getResource("ssaoBlur").withTexture(GL_RED).checkStatus();
	mGraph.getResource("ssaoBlur").unbind();

	// Shadow Maps
	mGraph.addResource(
		"directional", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height")));
	mGraph.getResource("directional").bind();
	mGraph.getResource("directional").withTextureDepth(GL_DEPTH_COMPONENT24, true).checkStatus();
	mGraph.getResource("directional").unbind();

	mGraph.addResource(
		"point", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height")));
	mGraph.getResource("point").bind();
	mGraph.getResource("point").withTextureCubemapDepthArray(
				CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_point"), GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mGraph.getResource("point").unbind();

	mGraph.addResource(
		"spot", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height")));
	mGraph.getResource("spot").bind();
	mGraph.getResource("spot").withTextureDepthArray(
				CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_spot"), GL_DEPTH_COMPONENT24, true)
			.checkStatus();

	// PostProcess Render Targets
	auto addPPRenderTarget = [&](const std::string& name) {
		mGraph.addResource(
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

void Renderer::refreshCameraData() const {
	struct alignas(16) PackedView {
		glm::mat4 view;
		glm::mat4 skyView;
		glm::vec4 viewPos;
	};

	const PackedView packed = {
		.view = mRenderCtx.camera->viewMatrix(),
		.skyView = glm::mat4(glm::mat3(mRenderCtx.camera->viewMatrix())),
		.viewPos = glm::vec4(mRenderCtx.camera->position(), 1.0)
	};

	mCameraUBO.bind();
	mCameraUBO.setData(&packed, sizeof(PackedView), 0);
}

void Renderer::sortEntities() {
	const glm::vec3& camPos = mRenderCtx.camera->position();

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
