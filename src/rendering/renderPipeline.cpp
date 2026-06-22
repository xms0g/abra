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
#include "mesh/mesh.h"
#include "mesh/vertex.hpp"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderFlags.hpp"
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
#include "mesh/vertexArray.h"
#include "../config/configManager.h"
#include "../core/camera.h"
#include "../ECS/registry.h"
#include "../ECS/components/bv.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/instance.hpp"
#include "../ECS/components/skybox.hpp"
#include "../math/boundingVolume.h"
#include "../math/matrix.h"
#include "../event/eventBus.hpp"
#include "../resourceManager/resourceManager.h"

RenderPipeline::RenderPipeline(Registry* registry, SDL_Window* window, SDL_GLContext context) {
	RequireComponent<MeshComponent>();
	RequireComponent<TransformComponent>();
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		throw std::runtime_error(std::string("ERROR::RENDERER::FAILED_TO_INIT_GLAD"));
	}

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	mRenderCtx = std::make_unique<RenderContext>();
	mRenderCtx->renderQueue = &mRenderQueue;

	mLightSystem = &registry->addSystem<LightSystem>();

	mRenderCtx->light.dirLights = &mLightSystem->dirLights();
	mRenderCtx->light.pointLights = &mLightSystem->pointLights();
	mRenderCtx->light.spotLights = &mLightSystem->spotLights();

	mShadowSystem = std::make_unique<ShadowSystem>();
	mSyncStateSystem = std::make_unique<SyncStateSystem>();

	GuiBackend::init(window, context, "#version 410");
}

RenderPipeline::~RenderPipeline() {
	GuiBackend::shutdown();
}

void RenderPipeline::configure(const Camera& camera, EventBus& eventBus) {
	mLightSystem->configure(*mRenderCtx, eventBus);
	mSyncStateSystem->configure(*mRenderCtx, eventBus);
	mShadowSystem->configure(*mRenderCtx, eventBus);

	// Create framebuffers
	int32_t width = ConfigManager::instance().get<int32_t>("window.width");
	int32_t height = ConfigManager::instance().get<int32_t>("window.height");

	mSceneBuffer = std::make_unique<FrameBuffer>(width, height);
#ifdef MSAA
	int32_t sampleCount = ConfigManager::instance().get<int32_t>("msaa.sample_count");
	glEnable(GL_MULTISAMPLE);
	mIntermediateBuffer = std::make_unique<FrameBuffer>(width, height);
# ifdef HDR
	mIntermediateBuffer->withTextureFP(GL_RGBA)
			.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureFPMultisampled(sampleCount, GL_RGBA)
# else
	mIntermediateBuffer->withTexture(GL_RGBA)
			.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureMultisampled(sampleCount, GL_RGBA)
# endif
			.withRenderBufferDepthMultisampled(sampleCount, GL_DEPTH_COMPONENT24)
#else
# ifdef HDR
	mSceneBuffer->withTextureFP(GL_RGBA)
# else
	mSceneBuffer->withTexture(GL_RGBA)
# endif
			.withTextureDepth(GL_DEPTH_COMPONENT24, false)
#endif
			.checkStatus();
	mSceneBuffer->unbind();

	// Create camera buffer
	mCameraUBO = std::make_unique<UniformBuffer>(
		DYNAMIC,
		3 * sizeof(glm::mat4) + sizeof(glm::vec4),
		ConfigManager::instance().get<uint32_t>("camera.ubo_binding"));

	// Create render passes
	mRenderPasses.emplace_back(std::make_unique<CullingPass>());

	if (!mRenderQueue.deferredGroups.empty()) {
		mRenderPasses.push_back(std::make_unique<DeferredGeometryPass>());
		mRenderPasses.push_back(std::make_unique<SSAOPass>());
		mRenderPasses.push_back(std::make_unique<DeferredLightingPass>());
	}

	if (!mRenderQueue.opaqueGroups.empty() || !mRenderQueue.blendGroups.empty()) {
		mRenderPasses.emplace_back(std::make_unique<ForwardPass>());
	}

	if (!mRenderQueue.debugGroups.empty()) {
		mRenderPasses.emplace_back(std::make_unique<DebugPass>());
	}

	if (!mRenderQueue.opaqueInstancedGroups.empty() || !mRenderQueue.blendInstancedGroups.empty()) {
		mRenderPasses.emplace_back(std::make_unique<InstancedPass>());
	}

	if (!mRenderQueue.terrain.empty()) {
		mRenderPasses.emplace_back(std::make_unique<TerrainPass>());
	}

	mRenderPasses.emplace_back(std::make_unique<SkyboxPass>());
#ifdef MSAA
	mRenderPasses.emplace_back(std::make_unique<ResolvePass>());
	mRenderCtx->intermediateBuffer = mIntermediateBuffer.get();
#endif
	mRenderPasses.emplace_back(std::make_unique<PostProcessPass>());

	mRenderCtx->sceneBuffer = mSceneBuffer.get();
	mRenderCtx->camera.self = &camera;
	mRenderCtx->PBR.irradianceMap = ResourceManager::instance().get<BaseFrameBuffer>("irradianceMap");
	mRenderCtx->PBR.prefilterMap = ResourceManager::instance().get<BaseFrameBuffer>("prefilterMap");
	mRenderCtx->PBR.brdfLUT = ResourceManager::instance().get<BaseFrameBuffer>("brdfLUT");

	// Set camera projection matrix
	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(camera.zoom()),
		static_cast<float>(width) / static_cast<float>(height),
		camera.znear(), camera.zfar());

	const glm::mat4 invProjectionMat = glm::inverse(projectionMat);

	mCameraUBO->bind();
	mCameraUBO->setData(glm::value_ptr(projectionMat), sizeof(glm::mat4), sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO->setData(glm::value_ptr(invProjectionMat), sizeof(glm::mat4), 2 * sizeof(glm::mat4) + sizeof(glm::vec4));
	mCameraUBO->unbind();

	// Configure render passes
	for (const auto& pass: mRenderPasses) {
		pass->configure(*mRenderCtx, eventBus);
	}


	const std::vector<UniformBinding> uboBindings = {
		{ConfigManager::instance().get<std::string>("camera.block_name"), ConfigManager::instance().get<uint32_t>("camera.ubo_binding"), mCameraUBO.get(), &UniformBuffer::configure},
		{ConfigManager::instance().get<std::string>("light.block_name"), ConfigManager::instance().get<uint32_t>("light.ubo_binding"), mLightSystem->ubo(), &UniformBuffer::configure},
		{ConfigManager::instance().get<std::string>("shadow.block_name"), ConfigManager::instance().get<uint32_t>("shadow.ubo_binding"), mShadowSystem->ubo(), &UniformBuffer::configure}
	};

	const std::vector<TextureBinding> shadowMapBindings = {
		{"shadowMap", ConfigManager::instance().get<int32_t>("shadow.texture_slot")},
		{"shadowCubemap", ConfigManager::instance().get<int32_t>("shadow.texture_slot") + 1},
		{"persShadowMap", ConfigManager::instance().get<int32_t>("shadow.texture_slot") + 2}
	};

	// Configure shaders
	for (const auto& [name,shader]: ResourceManager::instance().getShaders()) {
		for (const auto& [blockName, binding, buffer, configure]: uboBindings) {
			std::invoke(configure, buffer, shader->id(), binding, blockName.c_str());
		}

		RenderCommand::setTextureUnits(shadowMapBindings, *shader);
	}
}

void RenderPipeline::batchEntities() {
	for (const auto& entity: getSystemEntities()) {
		batchEntity(entity);
	}
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

void RenderPipeline::batchEntity(const Entity& entity) {
	static uint32_t materialIndex{0}, textureOffset{0}, meshIndex{0};

	auto pos = entity.getComponent<TransformComponent>().position;
	auto rot = entity.getComponent<TransformComponent>().rotation;
	auto scale = entity.getComponent<TransformComponent>().scale;
	auto modelMat = math::modelMatrix(pos, rot, scale);
	auto normalMat = math::normalMatrix(modelMat);

	mRenderQueue.entity.positions.push_back(pos);
	mRenderQueue.entity.rotations.push_back(rot);
	mRenderQueue.entity.scales.push_back(scale);
	mRenderQueue.entity.models.push_back(modelMat);
	mRenderQueue.entity.normals.push_back(normalMat);

	mRenderQueue.entity.centers.push_back(
		!entity.hasComponent<SkyboxComponent>()
			? entity.getComponent<BoundingVolumeComponent>().bv->center()
			: glm::vec3(0.0f)
	);
	mRenderQueue.entity.extents.push_back(
		!entity.hasComponent<SkyboxComponent>()
			? entity.getComponent<BoundingVolumeComponent>().bv->extents()
			: glm::vec3(0.0f)
	);

	mRenderQueue.entity.debugModes.emplace_back(0);
	mRenderQueue.entity.heightScales.emplace_back(entity.getComponent<MaterialComponent>().heightScale);

	auto& matComponent = entity.getComponent<MaterialComponent>();

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		std::vector<uint32_t> meshIndices;

		for (const auto& mesh: meshes) {
			meshIndices.push_back(meshIndex++);
			mRenderQueue.mesh.vaos.push_back(mesh.vao().id());
			mRenderQueue.mesh.vertexCounts.push_back(mesh.vertices().size());
			mRenderQueue.mesh.indexCounts.push_back(mesh.indices().size());
			mRenderQueue.mesh.maxCounts.push_back(mesh.max());
			mRenderQueue.mesh.minCounts.push_back(mesh.min());
		}

		auto& material = matComponent.materials->at(matID);
		material.idx = materialIndex++;

		mRenderQueue.material.flags.emplace_back(material.flags);
		mRenderQueue.material.alphaCutoffs.emplace_back(material.alphaCutoff);
		mRenderQueue.material.colors.emplace_back(material.color);

		for (const auto& texture: material.textures) {
			mRenderQueue.material.textures.push_back(texture.id);
		}

		size_t textureCount = material.textures.size();
		MaterialBatch matBatch{material.idx, textureOffset, textureCount, nullptr, meshIndices};
		textureOffset += textureCount;

		// Set shader
		if (material.flags & OPAQUE) {
			if (material.flags & UNLIT) {
				matBatch.shader = ResourceManager::instance().get<Shader>("unlit");
			} else {
				if (entity.hasComponent<InstanceComponent>()) {
					matBatch.shader = ResourceManager::instance().get<Shader>("instancedOpaque");
				} else {
					matBatch.shader = ResourceManager::instance().get<Shader>("opaque");
				}
			}
		} else if (material.flags & BLEND) {
			if (entity.hasComponent<InstanceComponent>()) {
				matBatch.shader = ResourceManager::instance().get<Shader>("instancedBlend");
			} else {
				matBatch.shader = ResourceManager::instance().get<Shader>("blend");
			}
		} else if (material.flags & CUBEMAP) {
			matBatch.shader = ResourceManager::instance().get<Shader>("skybox");
		} else if (material.flags & TERRAIN) {
			matBatch.shader = ResourceManager::instance().get<Shader>("terrain");
		}

		RenderGroup group;
		InstanceGroup instance;

		if (entity.hasComponent<InstanceComponent>()) {
			const auto& instComponent = entity.getComponent<InstanceComponent>();
			instance = {entity.id(), matBatch, *instComponent.transforms};
		} else {
			group = {entity.id(), matBatch};
		}

		if (entity.hasComponent<DebugComponent>()) {
			mRenderQueue.debugGroups.push_back(group);
		}

		if (material.flags & CASTSHADOW) {
			mRenderQueue.shadowGroups.push_back(group);
		}

		if (material.flags & PBR) {
			mRenderQueue.deferredGroups.push_back(group);
		} else if (material.flags & OPAQUE) {
			if (entity.hasComponent<InstanceComponent>()) {
				mRenderQueue.opaqueInstancedGroups.push_back(instance);
			} else {
				mRenderQueue.opaqueGroups.push_back(group);
			}
		} else if (material.flags & BLEND) {
			if (entity.hasComponent<InstanceComponent>()) {
				mRenderQueue.blendInstancedGroups.push_back(instance);
			} else {
				mRenderQueue.blendGroups.push_back(group);
			}
		} else if (material.flags & CUBEMAP) {
			mRenderQueue.skybox.push_back(group);
		} else if (material.flags & TERRAIN) {
			mRenderQueue.terrain.push_back(group);
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
				const auto aPos = mRenderQueue.entity.positions[a.entityID];
				const auto bPos = mRenderQueue.entity.positions[b.entityID];

				const float da = glm::length2(camPos - aPos);
				const float db = glm::length2(camPos - bPos);

				if (transparent)
					return da > db;
				return da < db;
			}
		);
	};

	// Sort opaque objects front to back
	sortBatches(mRenderQueue.deferredGroups, false);
	sortBatches(mRenderQueue.opaqueGroups, false);
	sortBatches(mRenderQueue.blendGroups, true);

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
