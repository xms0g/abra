#include "renderPipeline.h"
#include <SDL.h>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "shader.h"
#include "renderCommand.h"
#include "systems/lightSystem.h"
#include "systems/syncStateSystem.h"
#include "systems/shadowSystem/shadowSystem.h"
#include "buffers/frameBuffer.h"
#include "buffers/uniformBuffer.h"
#include "models/quad.h"
#include "mesh/mesh.h"
#include "mesh/vertex.hpp"
#include "mesh/vertexArray.h"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderFlags.hpp"
#include "renderContext/renderGroup.hpp"
#include "renderContext/renderableObject.hpp"
#include "renderContext/renderQueue.hpp"
#include "renderContext/renderData.hpp"
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
#include "../ECS/components/bv.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/instance.hpp"
#include "../math/boundingVolume.h"
#include "../math/matrix.h"
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
	batchEntities();
	createRenderPasses(eventBus);
	createUniformBuffers(camera);
	configureSystems(eventBus);
	configureShaders();
}

void RenderPipeline::render() {
	GuiBackend::newFrame();
	refreshCameraData();
	sortEntities();

	mRenderCtx->materialCache.reset();

	mSceneBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (const auto& pass: mRenderPasses) {
		pass->execute(*mRenderCtx);
	}

	mRenderCtx->sceneBuffer = mSceneBuffer.get();
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
	mCameraUBO = std::make_unique<UniformBuffer>(
		DYNAMIC,
		3 * sizeof(glm::mat4) + sizeof(glm::vec4),
		cfg.get<uint32_t>("camera.ubo_binding"));

	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(camera.zoom()),
		static_cast<float>(cfg.get<int32_t>("window.width")) / static_cast<float>(cfg.get<int32_t>("window.height")),
		camera.znear(), camera.zfar());

	const glm::mat4 invProjectionMat = glm::inverse(projectionMat);

	mCameraUBO->setData(glm::value_ptr(projectionMat), sizeof(glm::mat4), sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO->setData(glm::value_ptr(invProjectionMat), sizeof(glm::mat4), 2 * sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO->unbind();
}

void RenderPipeline::createRenderQueues() {
	mRenderQueue = std::make_unique<RenderQueue>();

	mRenderQueue->set("opaqueInstanced", std::vector<InstanceGroup>());
	mRenderQueue->set("blendInstanced", std::vector<InstanceGroup>());
	mRenderQueue->set("opaque", std::vector<RenderGroup>());
	mRenderQueue->set("blend", std::vector<RenderGroup>());
	mRenderQueue->set("debug", std::vector<RenderGroup>());
	mRenderQueue->set("shadow", std::vector<RenderGroup>());
	mRenderQueue->set("terrain", std::vector<RenderGroup>());
	mRenderQueue->set("skybox", std::vector<RenderGroup>());
	mRenderQueue->set("deferred", std::vector<RenderGroup>());
	mRenderQueue->set("visibleDeferred", std::vector<RenderableObject>());
	mRenderQueue->set("visibleOpaque", std::vector<RenderableObject>());
	mRenderQueue->set("visibleBlend", std::vector<RenderableObject>());
	mRenderQueue->set("visibleDebug", std::vector<RenderableObject>());
}

void RenderPipeline::createRenderPasses(EventBus& eventBus) {
	mRenderPasses.emplace_back(std::make_unique<CullingPass>());

	if (!mRenderQueue->get<std::vector<RenderGroup> >("deferred").empty()) {
		mRenderPasses.emplace_back(std::make_unique<DeferredGeometryPass>());
		mRenderPasses.emplace_back(std::make_unique<SSAOPass>());
		mRenderPasses.emplace_back(std::make_unique<DeferredLightingPass>());
	}

	if (!mRenderQueue->get<std::vector<RenderGroup> >("opaque").empty() ||
		!mRenderQueue->get<std::vector<RenderGroup> >("blend").empty()) {
		mRenderPasses.emplace_back(std::make_unique<ForwardPass>());
		}

	if (!mRenderQueue->get<std::vector<RenderGroup> >("debug").empty()) {
		mRenderPasses.emplace_back(std::make_unique<DebugPass>());
	}

	if (!mRenderQueue->get<std::vector<InstanceGroup> >("opaqueInstanced").empty() ||
		!mRenderQueue->get<std::vector<InstanceGroup> >("blendInstanced").empty()) {
		mRenderPasses.emplace_back(std::make_unique<InstancedPass>());
		}

	if (!mRenderQueue->get<std::vector<RenderGroup> >("terrain").empty()) {
		mRenderPasses.emplace_back(std::make_unique<TerrainPass>());
	}

	mRenderPasses.emplace_back(std::make_unique<SkyboxPass>());

	if (cfg.get<bool>("msaa.enabled")) {
		mRenderPasses.emplace_back(std::make_unique<ResolvePass>());
		mRenderCtx->intermediateBuffer = mIntermediateBuffer.get();
	}

	mRenderPasses.emplace_back(std::make_unique<PostProcessPass>());

	for (const auto& pass: mRenderPasses) {
		pass->configure(*mRenderCtx, eventBus);
	}
}

void RenderPipeline::createFrameBuffers() {
	mSceneBuffer = std::make_unique<FrameBuffer>(
		cfg.get<int32_t>("window.width"),
		cfg.get<int32_t>("window.height"));

	if (cfg.get<bool>("msaa.enabled")) {
		glEnable(GL_MULTISAMPLE);
		const int32_t sampleCount = cfg.get<int32_t>("msaa.sample_count");
		mIntermediateBuffer = std::make_unique<FrameBuffer>(
			cfg.get<int32_t>("window.width"),
			cfg.get<int32_t>("window.height"));

		if (cfg.get<bool>("hdr.enabled")) {
			mIntermediateBuffer->withTextureFP(GL_RGBA)
					.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
					.checkStatus();

			mSceneBuffer->bind();
			mSceneBuffer->withTextureFPMultisampled(sampleCount, GL_RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, GL_DEPTH_COMPONENT24)
					.checkStatus();
		} else {
			mIntermediateBuffer->withTexture(GL_RGBA)
					.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
					.checkStatus();

			mSceneBuffer->bind();
			mSceneBuffer->withTextureMultisampled(sampleCount, GL_RGBA)
					.withRenderBufferDepthMultisampled(sampleCount, GL_DEPTH_COMPONENT24)
					.checkStatus();
		}
	} else {
		if (cfg.get<bool>("hdr.enabled")) {
			mSceneBuffer->withTextureFP(GL_RGBA)
					.withTextureDepth(GL_DEPTH_COMPONENT24, false)
					.checkStatus();
		} else {
			mSceneBuffer->withTexture(GL_RGBA)
					.withTextureDepth(GL_DEPTH_COMPONENT24, false)
					.checkStatus();
		}
	}
	mSceneBuffer->unbind();
}

void RenderPipeline::createRenderContext(const Camera& camera) {
	mRenderCtx = std::make_unique<RenderContext>();
	mRenderData = std::make_unique<RenderData>();
	mRenderCtx->renderData = mRenderData.get();
	mRenderCtx->renderQueue = mRenderQueue.get();
	mRenderCtx->light.dirLights = &mLightSystem->dirLights();
	mRenderCtx->light.pointLights = &mLightSystem->pointLights();
	mRenderCtx->light.spotLights = &mLightSystem->spotLights();
	mRenderCtx->sceneBuffer = mSceneBuffer.get();
	mRenderCtx->camera.self = &camera;
}

void RenderPipeline::configureSystems(EventBus& eventBus) const {
	mLightSystem->configure(*mRenderCtx, eventBus);
	mSyncStateSystem->configure(*mRenderCtx, eventBus);
	mShadowSystem->configure(*mRenderCtx, eventBus);
}

void RenderPipeline::configureShaders() {
	const UniformBinding uboBindings[] = {
		{cfg.get<std::string>("camera.block_name"), cfg.get<uint32_t>("camera.ubo_binding"), UniformBuffer::configure},
		{cfg.get<std::string>("light.block_name"), cfg.get<uint32_t>("light.ubo_binding"), UniformBuffer::configure},
		{cfg.get<std::string>("shadow.block_name"), cfg.get<uint32_t>("shadow.ubo_binding"), UniformBuffer::configure}
	};

	const TextureBinding shadowMapBindings[] = {
		{"shadowMap", cfg.get<int32_t>("shadow.texture_slot")},
		{"shadowCubemap", cfg.get<int32_t>("shadow.texture_slot") + 1},
		{"persShadowMap", cfg.get<int32_t>("shadow.texture_slot") + 2}
	};

	// Configure shaders
	for (const auto& [name,shader]: rm.getShaders()) {
		for (const auto& [blockName, binding, configure]: uboBindings) {
			configure(shader->id(), binding, blockName.c_str());
		}

		RenderCommand::setTextureUnits(shadowMapBindings, *shader);
	}
}

void RenderPipeline::refreshCameraData() const {
	mRenderCtx->camera.skyView = glm::mat4(glm::mat3(mRenderCtx->camera.self->viewMatrix()));

	struct alignas(16) PackedView {
		glm::mat4 view;
		glm::vec4 viewPos;
	};

	const PackedView packed = {
		mRenderCtx->camera.self->viewMatrix(),
		glm::vec4(mRenderCtx->camera.self->position(), 1.0)
	};

	mCameraUBO->bind();
	mCameraUBO->setData(&packed, sizeof(PackedView), 0);
}

void RenderPipeline::batchEntities() const {
	for (const auto& entity: getSystemEntities()) {
		batchEntity(entity);
	}
}

void RenderPipeline::batchEntity(const Entity& entity) const {
	static uint32_t materialIndex{0}, textureOffset{0}, meshIndex{0};

	const auto& transform = entity.getComponent<TransformComponent>();
	const auto modelMat = math::modelMatrix(transform.position, transform.rotation, transform.scale);
	const auto normalMat = math::normalMatrix(modelMat);

	mRenderData->entity.positions.push_back(transform.position);
	mRenderData->entity.rotations.push_back(transform.rotation);
	mRenderData->entity.scales.push_back(transform.scale);
	mRenderData->entity.models.push_back(modelMat);
	mRenderData->entity.normals.push_back(normalMat);

	const auto& bv = entity.getComponent<BoundingVolumeComponent>().bv;
	mRenderData->entity.centers.push_back(bv->center());
	mRenderData->entity.extents.push_back(bv->extents());

	mRenderData->entity.debugModes.emplace_back(0);

	auto& matComponent = entity.getComponent<MaterialComponent>();
	mRenderData->entity.heightScales.emplace_back(matComponent.heightScale);


	struct PassRule {
		uint32_t flags;
		std::string queue;
		std::string instancedQueue;
	} rules[] = {
		{CASTSHADOW, "shadow", "shadow"},
		{PBR, "deferred", "deferred"},
		{OPAQUE, "opaque", "opaqueInstanced"},
		{BLEND, "blend", "blendInstanced"},
	};

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		std::vector<uint32_t> meshIndices;

		for (const auto& mesh: meshes) {
			meshIndices.push_back(meshIndex++);
			mRenderData->mesh.vaos.push_back(mesh.vao().id());
			mRenderData->mesh.vertexCounts.push_back(mesh.vertices().size());
			mRenderData->mesh.indexCounts.push_back(mesh.indices().size());
			mRenderData->mesh.maxCounts.push_back(mesh.max());
			mRenderData->mesh.minCounts.push_back(mesh.min());
		}

		auto& material = matComponent.materials->at(matID);
		material.idx = materialIndex++;

		mRenderData->material.flags.push_back(material.flags);
		mRenderData->material.textureTargets.push_back(material.textureTarget);
		mRenderData->material.alphaCutoffs.push_back(material.alphaCutoff);
		mRenderData->material.colors.push_back(material.color);

		for (const auto& texture: material.textures) {
			mRenderData->material.textures.push_back(texture.id);
		}

		const size_t textureCount = material.textures.size();
		MaterialBatch matBatch{material.idx, textureOffset, textureCount, nullptr, meshIndices};
		textureOffset += textureCount;

		// Set shader
		matBatch.shader = material.shader;

		RenderGroup group;
		InstanceGroup instance;

		const bool isInstanced = matComponent.renderFlag == INSTANCED_PASS;

		if (isInstanced) {
			const auto& instComponent = entity.getComponent<InstanceComponent>();
			instance = {entity.id(), matBatch, *instComponent.transforms};
		} else {
			group = {entity.id(), matBatch};
		}

		if (matComponent.renderFlag == SKYBOX_PASS) {
			mRenderQueue->get<std::vector<RenderGroup> >("skybox").push_back(group);
		} else if (matComponent.renderFlag == TERRAIN_PASS) {
			mRenderQueue->get<std::vector<RenderGroup> >("terrain").push_back(group);
		}

		if (entity.hasComponent<DebugComponent>()) {
			mRenderQueue->get<std::vector<RenderGroup> >("debug").push_back(group);
		}

		for (const auto& [flags, queue, instancedQueue]: rules) {
			if (material.flags & flags) {
				if (isInstanced) {
					mRenderQueue->get<std::vector<InstanceGroup> >(instancedQueue).push_back(instance);
				} else {
					mRenderQueue->get<std::vector<RenderGroup> >(queue).push_back(group);
				}
			}
		}
	}
}

void RenderPipeline::sortEntities() {
	const glm::vec3& camPos = mRenderCtx->camera.self->position();

	auto sortBatches = [&](auto& batch, bool transparent) -> void {
		std::sort(
			batch.begin(),
			batch.end(),
			[&camPos, &transparent, this](const RenderGroup& a, const RenderGroup& b) {
				const auto aPos = mRenderData->entity.positions[a.entityID];
				const auto bPos = mRenderData->entity.positions[b.entityID];

				const float da = glm::length2(camPos - aPos);
				const float db = glm::length2(camPos - bPos);

				if (transparent)
					return da > db;
				return da < db;
			}
		);
	};

	// Sort opaque objects front to back
	sortBatches(mRenderQueue->get<std::vector<RenderGroup> >("deferred"), false);
	sortBatches(mRenderQueue->get<std::vector<RenderGroup> >("opaque"), false);
	sortBatches(mRenderQueue->get<std::vector<RenderGroup> >("blend"), true);

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
