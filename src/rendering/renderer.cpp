#include "renderer.h"
#include <SDL.h>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "command.hpp"
#include "batcher.h"
#include "systems/lightSystem.h"
#include "systems/syncStateSystem.h"
#include "systems/shadowSystem/shadowSystem.h"
#include "buffers/frameBuffer.h"
#include "buffers/uniformBuffer.h"
#include "mesh/mesh.h"
#include "mesh/vertexArray.h"
#include "models/cube.h"
#include "models/quad.h"
#include "context/renderGroup.hpp"
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
#include "passes/forwardUnlit.h"
#include "passes/forwardBlend.h"
#include "passes/instancedBlend.h"
#include "passes/postProcess/postProcess.h"
#include "gui/backend.h"
#include "material/material.hpp"
#include "../config/configManager.h"
#include "../core/camera.h"
#include "../core/window.h"
#include "../ECS/registry.h"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/material.hpp"
#include "../event/eventBus.hpp"

Renderer::Renderer(Registry& registry, const Camera& camera, Window& window) {
	RequireComponent<MeshComponent>();
	RequireComponent<TransformComponent>();
	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		throw std::runtime_error(std::string("ERROR::RENDERER::FAILED_TO_INIT_GLAD"));
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	createRenderQueues();
	createSystems(registry);
	createFrameBuffers();

	mRenderCtx.renderData = &mRenderData;
	mRenderCtx.queueRegistry = &mQueueRegistry;
	mRenderCtx.pbrBuffers = &mPBRBuffers;
	mRenderCtx.camera = &camera;

	mEncoder = GraphicsEncoder{};
	GuiBackend::init(&*window, window.glContext(), "#version 410");
}

Renderer::~Renderer() {
	GuiBackend::shutdown();
}

void Renderer::configure(EventBus& eventBus) {
	createPBRBuffers();

	Batcher batcher;
	batcher.build(mRenderData, mQueueRegistry, getSystemEntities());

	createRenderPasses(eventBus);
	createUniformBuffers();
	configureSystems(eventBus);
}

void Renderer::render() {
	GuiBackend::newFrame();
	updateUniformBuffers();
	sortEntities();

	mEncoder.reset();

	auto& frameBuffer = mGraph.getResource(mIndexes.sceneBuffer);
	mEncoder.beginRendering({
		.frameBuffer = frameBuffer,
		.clearColor = true,
		.clearDepth = true,
		.viewport = {.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()}
	});

	mGraph.execute(mRenderCtx, mEncoder);
}

void Renderer::drawGui() {
	GuiBackend::renderFrame();
}

void Renderer::createSystems(Registry& registry) {
	mLightSystem = &registry.addSystem<LightSystem>();
	mShadowSystem = std::make_unique<ShadowSystem>();
	mSyncStateSystem = std::make_unique<SyncStateSystem>();
}

void Renderer::createUniformBuffers() {
	// Create camera buffer
	mCameraUBO = UniformBuffer{DYNAMIC, sizeof(UniformBufferObject)};

	DescriptorSet cameraSet{};
	cameraSet.write(
		CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
		{.id = mCameraUBO.id(), .target = mCameraUBO.target(), .size = sizeof(UniformBufferObject)}
	);

	mEncoder.bindDescriptorSet(cameraSet);
}

void Renderer::createPBRBuffers() {
	const auto entityIt = std::ranges::find_if(getSystemEntities(), [](const Entity& e) {
		return e.name() == "Skybox";
	});

	const auto& matComponent = entityIt->getComponent<MaterialComponent>();
	auto& textures = matComponent.materials->at(0).textures;
	const auto& texture = textures.front();

	if (texture.target == TextureTarget::Texture2D) {
		const TextureView view = createEnvMap(texture);
		textures.clear();
		textures.emplace_back(view.id, 0, view.target, "");
	}

	createIrradianceMap({.id = texture.id, .target = texture.target});
	createPrefilterMap({.id = texture.id, .target = texture.target});
	createBrdfLUT();
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
	mQueueRegistry.set<DrawCommand>("TerrainCommands");
	mQueueRegistry.set<DrawCommand>("SkyboxCommands");
}

void Renderer::createRenderPasses(EventBus& eventBus) {
	mGraph.addPass(
		"DeferredLightingPass",
		std::make_unique<DeferredLightingPass>(),
		{"gBuffer", "ssaoBlur"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("deferred")
	);

	mGraph.addPass(
		"DeferredGeometryPass",
		std::make_unique<DeferredGeometryPass>(),
		{"DeferredCommands"},
		{"gBuffer"},
		!mQueueRegistry.empty("deferred")
	);
	mGraph.addPass(
		"ForwardOpaquePass",
		std::make_unique<ForwardOpaquePass>(),
		{"sceneBuffer", "OpaqueCommands"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("opaque")
	);
	mGraph.addPass(
		"ForwardBlendPass",
		std::make_unique<ForwardBlendPass>(),
		{"sceneBuffer", "BlendCommands"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("blend")
	);
	mGraph.addPass(
		"ForwardUnlitPass",
		std::make_unique<ForwardUnlitPass>(),
		{"sceneBuffer", "UnlitCommands"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("unlit")
	);
	mGraph.addPass(
		"SSAOPass",
		std::make_unique<SSAOPass>(),
		{"gBuffer"},
		{"ssao", "ssaoBlur"},
		!mQueueRegistry.empty("deferred")
	);
	mGraph.addPass(
		"InstancedOpaquePass",
		std::make_unique<InstancedOpaquePass>(),
		{"sceneBuffer"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("opaqueInstanced")
	);
	mGraph.addPass(
		"InstancedBlendPass",
		std::make_unique<InstancedBlendPass>(),
		{"sceneBuffer"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("blendInstanced")
	);
	mGraph.addPass(
		"DebugPass",
		std::make_unique<DebugPass>(),
		{"sceneBuffer", "DebugCommands"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("debug")
	);
	mGraph.addPass(
		"TerrainPass",
		std::make_unique<TerrainPass>(),
		{"sceneBuffer"},
		{"sceneBuffer"},
		!mQueueRegistry.empty("terrain")
	);
	mGraph.addPass(
		"ResolvePass",
		std::make_unique<ResolvePass>(),
		{"sceneBuffer"},
		{"intermediateBuffer"},
		CONFIG_MANAGER.get<bool>("msaa.enabled"));
	mGraph.addPass(
		"SkyboxPass",
		std::make_unique<SkyboxPass>(),
		{"sceneBuffer"},
		{"sceneBuffer"},
		true);
	mGraph.addPass(
		"CullingPass",
		std::make_unique<CullingPass>(),
		{},
		{"OpaqueCommands", "UnlitCommands", "BlendCommands", "DeferredCommands", "DebugCommands"},
		true);
	mGraph.addPass(
		"PostProcessPass",
		std::make_unique<PostProcessPass>(),
		{"sceneBuffer"},
		{"frameBuffer"},
		true);

	mGraph.compile();
	mGraph.configure(mRenderCtx, mEncoder, eventBus);
}

void Renderer::createFrameBuffers() {
	int32_t width = CONFIG_MANAGER.get<int32_t>("window.width");
	int32_t height = CONFIG_MANAGER.get<int32_t>("window.height");

	auto& sceneBuffer = mGraph.addResource("sceneBuffer", std::make_unique<FrameBuffer>(width, height));
	const uint32_t sceneBufferID = mGraph.getResourceID("sceneBuffer");
	mIndexes.sceneBuffer = sceneBufferID;

	if (CONFIG_MANAGER.get<bool>("msaa.enabled")) {
		glEnable(GL_MULTISAMPLE);
		const int32_t sampleCount = CONFIG_MANAGER.get<int32_t>("msaa.sample_count");

		auto& intermediateBuffer = mGraph.addResource("intermediateBuffer",
		                                              std::make_unique<FrameBuffer>(width, height));

		mEncoder.bindFrameBuffer(intermediateBuffer);

		if (CONFIG_MANAGER.get<bool>("hdr.enabled")) {
			intermediateBuffer.withTextureFP(BaseFormat::RGBA)
					.withRenderBufferDepth(InternalFormat::Depth24)
					.checkStatus();

			mEncoder.bindFrameBuffer(sceneBuffer);
			sceneBuffer.withTextureFPMultisampled(sampleCount, BaseFormat::RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, InternalFormat::Depth24)
					.checkStatus();
		} else {
			intermediateBuffer.withTexture(BaseFormat::RGBA)
					.withRenderBufferDepth(InternalFormat::Depth24)
					.checkStatus();

			mEncoder.bindFrameBuffer(sceneBuffer);
			sceneBuffer.withTextureMultisampled(sampleCount, BaseFormat::RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, InternalFormat::Depth24)
					.checkStatus();
		}
	} else {
		mEncoder.bindFrameBuffer(sceneBuffer);
		if (CONFIG_MANAGER.get<bool>("hdr.enabled")) {
			sceneBuffer.withTextureFP(BaseFormat::RGBA)
					.withTextureDepth(InternalFormat::Depth24, false)
					.checkStatus();
		} else {
			sceneBuffer.withTexture(BaseFormat::RGBA)
					.withTextureDepth(InternalFormat::Depth24, false)
					.checkStatus();
		}
	}

	// GBuffer
	auto& gBuffer = mGraph.addResource("gBuffer", std::make_unique<FrameBuffer>(width, height));

	mEncoder.bindFrameBuffer(gBuffer);
	gBuffer.withTextureFP(BaseFormat::RGBA) // position
			.withTextureFP(BaseFormat::RGBA); // normal
	if (CONFIG_MANAGER.get<bool>("hdr.enabled")) {
		// albedo
		gBuffer.withTextureFP(BaseFormat::RGBA);
	} else {
		gBuffer.withTexture(BaseFormat::RGBA);
	}
	// Emissive placed into alpha channels in position, normal, albedo

	gBuffer.withTexture(BaseFormat::RGBA) // orm
			.configureAttachments()
			.withTextureDepth(InternalFormat::Depth24, false)
			.checkStatus();

	// SSAO
	auto& ssao = mGraph.addResource("ssao", std::make_unique<FrameBuffer>(width / 2, height / 2));

	mEncoder.bindFrameBuffer(ssao);
	ssao.withTexture(BaseFormat::Red).checkStatus();

	auto& ssaoBlur = mGraph.addResource("ssaoBlur", std::make_unique<FrameBuffer>(width / 2, height / 2));

	mEncoder.bindFrameBuffer(ssaoBlur);
	ssaoBlur.withTexture(BaseFormat::Red).checkStatus();

	// Shadow Maps
	auto& directional = mGraph.addResource("directional", std::make_unique<FrameBuffer>(
		                                       CONFIG_MANAGER.get<int32_t>("shadow.map.width"),
		                                       CONFIG_MANAGER.get<int32_t>("shadow.map.height")));

	mEncoder.bindFrameBuffer(directional);
	directional.withTextureDepth(InternalFormat::Depth24, true).checkStatus();

	auto& point = mGraph.addResource("point", std::make_unique<FrameBuffer>(
		                                 CONFIG_MANAGER.get<int32_t>("shadow.map.width"),
		                                 CONFIG_MANAGER.get<int32_t>("shadow.map.height")));

	mEncoder.bindFrameBuffer(point);
	point.withTextureCubemapDepthArray(
				CONFIG_MANAGER.get<int32_t>("light.max_point"), InternalFormat::Depth24, true)
			.checkStatus();

	auto& spot = mGraph.addResource("spot", std::make_unique<FrameBuffer>(
		                                CONFIG_MANAGER.get<int32_t>("shadow.map.width"),
		                                CONFIG_MANAGER.get<int32_t>("shadow.map.height")));

	mEncoder.bindFrameBuffer(spot);
	spot.withTextureDepthArray(
				CONFIG_MANAGER.get<int32_t>("light.max_spot"), InternalFormat::Depth24, true)
			.checkStatus();

	// PostProcess Render Targets
	auto addPPRenderTarget = [&](const std::string& name) {
		auto& target = mGraph.addResource(name, std::make_unique<FrameBuffer>(width, height));

		mEncoder.bindFrameBuffer(target);
		if (CONFIG_MANAGER.get<bool>("hdr.enabled")) {
			target.withTextureFP(BaseFormat::RGBA);
		} else {
			target.withTexture(BaseFormat::RGBA);
		}
		target.checkStatus();
	};

	addPPRenderTarget("ping");
	addPPRenderTarget("pong");
	addPPRenderTarget("bloomPing");
	addPPRenderTarget("bloomPong");
}

void Renderer::configureSystems(EventBus& eventBus) {
	mLightSystem->configure(mRenderCtx, mEncoder, eventBus);
	mSyncStateSystem->configure(mRenderCtx, eventBus);
	mShadowSystem->configure(mRenderCtx, mGraph, mEncoder, eventBus);
}

void Renderer::updateUniformBuffers() const {
	const glm::mat4 projection = glm::perspective(
		glm::radians(mRenderCtx.camera->zoom()),
		CONFIG_MANAGER.get<float>("window.aspectRatio"),
		mRenderCtx.camera->znear(), mRenderCtx.camera->zfar());

	const UniformBufferObject ubo = {
		.view = mRenderCtx.camera->viewMatrix(),
		.inverseView = glm::inverse(mRenderCtx.camera->viewMatrix()),
		.skyView = glm::mat4(glm::mat3(mRenderCtx.camera->viewMatrix())),
		.cameraPos = glm::vec4(mRenderCtx.camera->position(), 1.0),
		.projection = projection
	};

	mCameraUBO.copyToMemory(&ubo, 0, sizeof(UniformBufferObject));
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

TextureView Renderer::createEnvMap(const Texture& hdrTexture) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Lequal,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("pbr/cubemap.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("pbr/equirectangularToCube.frag"), .stage = ShaderStageType::Fragment}
		}
	};

	DescriptorSetLayout materialLayout = {
		.bindings = {
			{.name = "equirectangularMap", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};

	PipelineLayout pipelineLayout = {.descriptorSets = {materialLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = pipelineLayout};
	auto pipeline = GraphicsPipeline{createInfo};

	auto encoder = GraphicsEncoder{};

	auto envMap = std::make_unique<FrameBuffer>(
		CONFIG_MANAGER.get<int32_t>("PBR.envMap.size"),
		CONFIG_MANAGER.get<int32_t>("PBR.envMap.size"));
	envMap->withTextureCubeMap()
			.withRenderBufferDepth(InternalFormat::Depth24)
			.checkStatus();

	mPBRBuffers.environment = std::move(envMap);

	Model::Cube cube;
	cube.meshes().at(0).front().uploadToGPU();
	const auto& cubeMesh = cube.meshes().at(0).front();

	DescriptorSet descriptorSet{};
	descriptorSet.write(0, {.id = hdrTexture.id, .target = hdrTexture.target});

	// convert HDR equirectangular environment map to cubemap equivalent
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.bindDescriptorSet(descriptorSet);

	encoder.bindFrameBuffer(*mPBRBuffers.environment);
	encoder.setViewport({
		.x = 0, .y = 0, .width = mPBRBuffers.environment->width(), .height = mPBRBuffers.environment->height()
	});

	for (int32_t i = 0; i < FACES; ++i) {
		mPBRBuffers.environment->attachTexture(0, Attachment::Color0, 0, i);
		encoder.setUniform("view", mCaptureViews[i]);

		encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

		encoder.bindVertexArray(cubeMesh.vao().id());
		encoder.drawIndexed(cubeMesh.indices().size());
	}

	encoder.unbindFrameBuffer();
	Texture::generateMipmaps(mPBRBuffers.environment->texture());

	return mPBRBuffers.environment->texture();
}

void Renderer::createIrradianceMap(const TextureView environment) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("pbr/cubemap.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("pbr/irradianceConv.frag"), .stage = ShaderStageType::Fragment}
		}
	};

	DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "environmentMap", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};

	PipelineLayout pipelineLayout = {.descriptorSets = {passLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = pipelineLayout};
	auto pipeline = GraphicsPipeline{createInfo};

	auto encoder = GraphicsEncoder{};

	auto irradianceMap = std::make_unique<FrameBuffer>(
		CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.size"),
		CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.size"));
	irradianceMap->withTextureCubeMap()
			.withRenderBufferDepth(InternalFormat::Depth24)
			.checkStatus();

	mPBRBuffers.irradiance = std::move(irradianceMap);

	Model::Cube cube;
	cube.meshes().at(0).front().uploadToGPU();
	const auto& cubeMesh = cube.meshes().at(0).front();

	DescriptorSet descriptorSet{};
	descriptorSet.write(0, environment);

	// solve diffuse integral by convolution to create an irradiance (cube)map.
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.bindDescriptorSet(descriptorSet);

	encoder.bindFrameBuffer(*mPBRBuffers.irradiance);
	encoder.setViewport({
		.x = 0, .y = 0, .width = mPBRBuffers.irradiance->width(), .height = mPBRBuffers.irradiance->height()
	});

	for (int32_t i = 0; i < FACES; ++i) {
		mPBRBuffers.irradiance->attachTexture(0, Attachment::Color0, 0, i);
		encoder.setUniform("view", mCaptureViews[i]);

		encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

		encoder.bindVertexArray(cubeMesh.vao().id());
		encoder.drawIndexed(cubeMesh.indices().size());
	}

	encoder.unbindFrameBuffer();
}

void Renderer::createPrefilterMap(const TextureView environment) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("pbr/cubemap.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("pbr/prefilter.frag"), .stage = ShaderStageType::Fragment}
		}
	};

	DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "environmentMap", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};

	PipelineLayout pipelineLayout = {.descriptorSets = {passLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = pipelineLayout};
	auto pipeline = GraphicsPipeline{createInfo};

	auto encoder = GraphicsEncoder{};

	auto prefilterMap = std::make_unique<FrameBuffer>(
		CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.size"),
		CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.size"));
	prefilterMap->withTextureCubeMap()
			.withRenderBufferDepth(InternalFormat::Depth24)
			.checkStatus();

	Texture::generateMipmaps(prefilterMap->texture());

	mPBRBuffers.prefilter = std::move(prefilterMap);

	Model::Cube cube;
	cube.meshes().at(0).front().uploadToGPU();
	const auto& cubeMesh = cube.meshes().at(0).front();

	DescriptorSet descriptorSet{};
	descriptorSet.write(0, environment);

	// run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.setUniform("resolution",
	                   static_cast<float>(CONFIG_MANAGER.get<int32_t>("PBR.envMap.size")));

	encoder.bindDescriptorSet(descriptorSet);

	encoder.bindFrameBuffer(*mPBRBuffers.prefilter);
	encoder.setViewport({
		.x = 0, .y = 0, .width = mPBRBuffers.prefilter->width(), .height = mPBRBuffers.prefilter->height()
	});

	constexpr int32_t mipLevels = 5;
	const int32_t prefilterMapSize = CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.size");

	for (int32_t i = 0; i < mipLevels; ++i) {
		const auto mipSize = static_cast<int32_t>(prefilterMapSize * std::pow(0.5, i));
		mPBRBuffers.prefilter->resizeRenderBuffer(mipSize, mipSize);

		const float roughness = static_cast<float>(i) / static_cast<float>(mipLevels - 1);
		encoder.setUniform("roughness", roughness);

		for (int32_t j = 0; j < FACES; ++j) {
			mPBRBuffers.prefilter->attachTexture(0, Attachment::Color0, i, j);
			encoder.setUniform("view", mCaptureViews[j]);

			encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

			encoder.bindVertexArray(cubeMesh.vao().id());
			encoder.drawIndexed(cubeMesh.indices().size());
		}
	}

	encoder.unbindFrameBuffer();
}

void Renderer::createBrdfLUT() {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("pbr/brdfLUT.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("pbr/brdfLUT.frag"), .stage = ShaderStageType::Fragment}
		}
	};

	DescriptorSetLayout passLayout = {};
	PipelineLayout pipelineLayout = {.descriptorSets = {passLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = pipelineLayout};
	auto pipeline = GraphicsPipeline{createInfo};

	auto encoder = GraphicsEncoder{};

	auto brdfLUT = std::make_unique<FrameBuffer>(
		CONFIG_MANAGER.get<int32_t>("PBR.brdfLUT.size"),
		CONFIG_MANAGER.get<int32_t>("PBR.brdfLUT.size"));
	brdfLUT->withTextureFP(BaseFormat::RG)
			.checkStatus();

	mPBRBuffers.brdfLUT = std::move(brdfLUT);

	const Model::Quad quad;
	// generate a 2D LUT from the BRDF equations used.
	encoder.bindPipeline(pipeline);
	encoder.bindFrameBuffer(*mPBRBuffers.brdfLUT);
	encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);
	encoder.setViewport(
		{.x = 0, .y = 0, .width = mPBRBuffers.brdfLUT->width(), .height = mPBRBuffers.brdfLUT->height()});

	encoder.bindVertexArray(quad.vao().id());
	encoder.draw(6);

	encoder.unbindFrameBuffer();
}
