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
#include "passes/postProcess/postProcess.h"
#include "gui/backend.h"
#include "material/material.hpp"
#include "../config/configManager.h"
#include "../core/camera.h"
#include "../core/window.h"
#include "../core/gui/ui.h"
#include "../ECS/registry.h"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/material.hpp"
#include "../event/eventBus.hpp"
#include "../resource/resourceManager.h"

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

void Renderer::configure(const Camera& camera, EventBus& eventBus) {
	createPBRBuffers();

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

	mEncoder.reset();

	auto& frameBuffer = mGraph.getResource("sceneBuffer");
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

void Renderer::createUniformBuffers(const Camera& camera) {
	// Create camera buffer
	mCameraUBO = UniformBuffer{
		DYNAMIC,
		4 * sizeof(glm::mat4) + sizeof(glm::vec4),
		CONFIG_MANAGER.get<int32_t>("camera.ubo_binding")
	};

	const float aspectRatio = static_cast<float>(CONFIG_MANAGER.get<int32_t>("window.width")) /
	                          static_cast<float>(CONFIG_MANAGER.get<int32_t>("window.height"));
	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(camera.zoom()),
		aspectRatio,
		camera.znear(), camera.zfar());

	mCameraUBO.bind();
	mCameraUBO.setData(glm::value_ptr(projectionMat), sizeof(glm::mat4), 3 * sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO.unbind();
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
		CONFIG_MANAGER.get<bool>("msaa.enabled"),
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
	mGraph.configure(mRenderCtx, mEncoder, eventBus);
}

void Renderer::createFrameBuffers() {
	int32_t width = CONFIG_MANAGER.get<int32_t>("window.width");
	int32_t height = CONFIG_MANAGER.get<int32_t>("window.height");

	mGraph.addResource("sceneBuffer", std::make_unique<FrameBuffer>(width, height));
	auto& sceneBuffer = mGraph.getResource("sceneBuffer");

	if (CONFIG_MANAGER.get<bool>("msaa.enabled")) {
		glEnable(GL_MULTISAMPLE);
		const int32_t sampleCount = CONFIG_MANAGER.get<int32_t>("msaa.sample_count");

		mGraph.addResource("intermediateBuffer", std::make_unique<FrameBuffer>(width, height));
		auto& intermediateBuffer = mGraph.getResource("intermediateBuffer");
		intermediateBuffer.bind();

		if (CONFIG_MANAGER.get<bool>("hdr.enabled")) {
			intermediateBuffer.withTextureFP(BaseFormat::RGBA)
					.withRenderBufferDepth(InternalFormat::Depth24)
					.checkStatus();

			sceneBuffer.bind();
			sceneBuffer.withTextureFPMultisampled(sampleCount, BaseFormat::RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, InternalFormat::Depth24)
					.checkStatus();
		} else {
			intermediateBuffer.withTexture(BaseFormat::RGBA)
					.withRenderBufferDepth(InternalFormat::Depth24)
					.checkStatus();

			sceneBuffer.bind();
			sceneBuffer.withTextureMultisampled(sampleCount, BaseFormat::RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, InternalFormat::Depth24)
					.checkStatus();
		}
	} else {
		sceneBuffer.bind();
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
	sceneBuffer.unbind();

	// GBuffer
	mGraph.addResource("gBuffer", std::make_unique<FrameBuffer>(width, height));
	auto& gBuffer = mGraph.getResource("gBuffer");
	gBuffer.bind();

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
	int32_t ssaoWidth = CONFIG_MANAGER.get<int32_t>("window.width") / 2;
	int32_t ssaoHeight = CONFIG_MANAGER.get<int32_t>("window.height") / 2;

	mGraph.addResource("ssao", std::make_unique<FrameBuffer>(ssaoWidth, ssaoHeight));
	mGraph.getResource("ssao").bind();
	mGraph.getResource("ssao").withTexture(BaseFormat::Red).checkStatus();
	mGraph.getResource("ssao").unbind();

	mGraph.addResource("ssaoBlur", std::make_unique<FrameBuffer>(ssaoWidth, ssaoHeight));
	mGraph.getResource("ssaoBlur").bind();
	mGraph.getResource("ssaoBlur").withTexture(BaseFormat::Red).checkStatus();
	mGraph.getResource("ssaoBlur").unbind();

	// Shadow Maps
	mGraph.addResource(
		"directional", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER.get<int32_t>("shadow.map_height")));
	mGraph.getResource("directional").bind();
	mGraph.getResource("directional").withTextureDepth(InternalFormat::Depth24, true).checkStatus();
	mGraph.getResource("directional").unbind();

	mGraph.addResource(
		"point", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER.get<int32_t>("shadow.map_height")));
	mGraph.getResource("point").bind();
	mGraph.getResource("point").withTextureCubemapDepthArray(
				CONFIG_MANAGER.get<int32_t>("light.max_point"), InternalFormat::Depth24, true)
			.checkStatus();
	mGraph.getResource("point").unbind();

	mGraph.addResource(
		"spot", std::make_unique<FrameBuffer>(
			CONFIG_MANAGER.get<int32_t>("shadow.map_width"),
			CONFIG_MANAGER.get<int32_t>("shadow.map_height")));
	mGraph.getResource("spot").bind();
	mGraph.getResource("spot").withTextureDepthArray(
				CONFIG_MANAGER.get<int32_t>("light.max_spot"), InternalFormat::Depth24, true)
			.checkStatus();

	// PostProcess Render Targets
	auto addPPRenderTarget = [&](const std::string& name) {
		mGraph.addResource(name, std::make_unique<FrameBuffer>(width, height));
		auto& target = mGraph.getResource(name);

		target.bind();
		if (CONFIG_MANAGER.get<bool>("hdr.enabled")) {
			target.withTextureFP(BaseFormat::RGBA);
		} else {
			target.withTexture(BaseFormat::RGBA);
		}
		target.checkStatus();
		target.unbind();
	};

	addPPRenderTarget("ping");
	addPPRenderTarget("pong");
	addPPRenderTarget("bloomPing");
	addPPRenderTarget("bloomPong");
}

void Renderer::configureSystems(EventBus& eventBus) {
	mLightSystem->configure(mRenderCtx, eventBus);
	mSyncStateSystem->configure(mRenderCtx, eventBus);
	mShadowSystem->configure(mRenderCtx, mGraph, mEncoder, eventBus);
}

void Renderer::refreshCameraData() const {
	struct alignas(16) PackedView {
		glm::mat4 view;
		glm::mat4 inverseView;
		glm::mat4 skyView;
		glm::vec4 cameraPos;
	};

	const PackedView packed = {
		.view = mRenderCtx.camera->viewMatrix(),
		.inverseView = glm::inverse(mRenderCtx.camera->viewMatrix()),
		.skyView = glm::mat4(glm::mat3(mRenderCtx.camera->viewMatrix())),
		.cameraPos = glm::vec4(mRenderCtx.camera->position(), 1.0)
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
		},
		.descriptors = {
			{.name = "equirectangularMap", .type = DescriptorType::Sampler2D, .binding = 0}
		},
	};

	auto pipeline = GraphicsPipeline{info};
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

	// convert HDR equirectangular environment map to cubemap equivalent
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.bindTexture({.id = hdrTexture.id, .target = hdrTexture.target}, 0);

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
		},
		.descriptors = {
			{.name = "environmentMap", .type = DescriptorType::SamplerCube, .binding = 0}
		},
	};

	auto pipeline = GraphicsPipeline{info};
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

	// solve diffuse integral by convolution to create an irradiance (cube)map.
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.bindTexture(environment, 0);

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
		},
		.descriptors = {
			{.name = "environmentMap", .type = DescriptorType::SamplerCube, .binding = 0}
		}
	};

	auto pipeline = GraphicsPipeline{info};
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

	// run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.setUniform("resolution",
	                   static_cast<float>(CONFIG_MANAGER.get<int32_t>("PBR.envMap.size")));

	encoder.bindTexture(environment, 0);

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
		},
		.descriptors = {
			{.name = "environmentMap", .type = DescriptorType::SamplerCube, .binding = 0}
		}
	};

	auto pipeline = GraphicsPipeline{info};
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
