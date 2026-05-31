#include "renderPipeline.h"
#include <SDL.h>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "shader.h"
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
#include "renderContext/instanceGroup.hpp"
#include "renderPasses/IRenderPass.hpp"
#include "renderPasses/deferredGeometryPass.h"
#include "renderPasses/deferredLightingPass.h"
#include "renderPasses/ssaoPass.h"
#include "renderPasses/debugPass.h"
#include "renderPasses/forwardPass.h"
#include "renderPasses/instancedPass.h"
#include "renderPasses/frustumCullingPass.h"
#include "renderPasses/skyboxPass.h"
#include "renderPasses/resolvePass.h"
#include "renderPasses/postProcess/postProcessPass.h"
#include "gui/backend.h"
#include "material/material.hpp"
#include "mesh/vertexArray.h"
#include "../config/config.hpp"
#include "../ECS/registry.h"
#include "../core/camera.h"
#include "../ECS/components/bv.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/instance.hpp"
#include "../ECS/components/skybox.hpp"
#include "../math/boundingVolume.h"
#include "../event/eventBus.hpp"
#include "../math/matrix.h"
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
	mRenderCtx->screen.width = SCR_WIDTH;
	mRenderCtx->screen.height = SCR_HEIGHT;
	mRenderCtx->renderQueue = &mRenderQueue;
	mRenderCtx->camera.ubo.binding = CAMERA_UBO_BINDING;
	mRenderCtx->camera.ubo.blockName = CAMERA_UBO_BLOCK_NAME;
	mRenderCtx->light.maxDirLights = MAX_DIRECTIONAL_LIGHTS;
	mRenderCtx->light.maxPointLights = MAX_POINT_LIGHTS;
	mRenderCtx->light.maxSpotLights = MAX_SPOT_LIGHTS;
	mRenderCtx->light.ubo.binding = LIGHT_UBO_BINDING;
	mRenderCtx->light.ubo.blockName = LIGHT_UBO_BLOCK_NAME;
	mRenderCtx->shadow.ubo.binding = SHADOW_UBO_BINDING;
	mRenderCtx->shadow.ubo.blockName = SHADOW_UBO_BLOCK_NAME;
	mRenderCtx->shadow.textureSlot = SHADOWMAP_TEXTURE_SLOT;
	mRenderCtx->shadow.width = SHADOWMAP_WIDTH;
	mRenderCtx->shadow.height = SHADOWMAP_HEIGHT;
	mRenderCtx->shadow.directional.maxLights = MAX_DIRECTIONAL_LIGHTS;
	mRenderCtx->shadow.directional.height = SHADOW_DIRECTIONAL_HEIGHT;
	mRenderCtx->shadow.directional.nearPlane = SHADOW_DIRECTIONAL_NEAR;
	mRenderCtx->shadow.directional.farPlane = SHADOW_DIRECTIONAL_FAR;
	mRenderCtx->shadow.directional.left = SHADOW_DIRECTIONAL_LEFT;
	mRenderCtx->shadow.directional.right = SHADOW_DIRECTIONAL_RIGHT;
	mRenderCtx->shadow.directional.bottom = SHADOW_DIRECTIONAL_BOTTOM;
	mRenderCtx->shadow.directional.top = SHADOW_DIRECTIONAL_TOP;
	mRenderCtx->shadow.omnidirectional.maxLights = MAX_POINT_LIGHTS;
	mRenderCtx->shadow.omnidirectional.nearPlane = SHADOW_OMNIDIRECTIONAL_NEAR;
	mRenderCtx->shadow.omnidirectional.farPlane = SHADOW_OMNIDIRECTIONAL_FAR;
	mRenderCtx->shadow.omnidirectional.fovy = SHADOW_OMNIDIRECTIONAL_FOVY;
	mRenderCtx->shadow.perspective.maxLights = MAX_SPOT_LIGHTS;
	mRenderCtx->shadow.perspective.nearPlane = SHADOW_PERSPECTIVE_NEAR;
	mRenderCtx->shadow.perspective.farPlane = SHADOW_PERSPECTIVE_FAR;
	mRenderCtx->ssao.ubo.binding = SSAO_UBO_BINDING;
	mRenderCtx->ssao.ubo.blockName = SSAO_UBO_BLOCK_NAME;
	mRenderCtx->ssao.kernelSize = SSAO_KERNEL_SIZE;
	mRenderCtx->ssao.noiseTextureSize = SSAO_NOISE_TEXTURE_SIZE;
	mRenderCtx->ssao.radius = SSAO_RADIUS;
	mRenderCtx->ssao.bias = SSAO_BIAS;
	mRenderCtx->ssao.intensity = SSAO_INTENSITY;
	mRenderCtx->ssao.textureSlot = SSAO_TEXTURE_SLOT;
	mRenderCtx->gBuffer.positionTextureIdx = G_POSITION_TEXTURE_IDX;
	mRenderCtx->gBuffer.normalTextureIdx = G_NORMAL_TEXTURE_IDX;
	mRenderCtx->gBuffer.albedoTextureIdx = G_ALBEDO_TEXTURE_IDX;
	mRenderCtx->gBuffer.ormTextureIdx = G_ORM_TEXTURE_IDX;
	mRenderCtx->gBuffer.depthTextureIdx = G_DEPTH_TEXTURE_IDX;

	registry->addSystem<LightSystem>();
	mLightSystem = &registry->getSystem<LightSystem>();

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
	mRenderCtx->shadow.directional.shader = ResourceManager::instance().getShader("depth");
	mRenderCtx->shadow.omnidirectional.shader = ResourceManager::instance().getShader("depthCubemap");
	mRenderCtx->shadow.perspective.shader = ResourceManager::instance().getShader("depth");
	mRenderCtx->gBuffer.shader = ResourceManager::instance().getShader("gBuffer");
	mRenderCtx->PBR.shader = ResourceManager::instance().getShader("deferredLighting");
	mRenderCtx->ssao.shader = ResourceManager::instance().getShader("ssao");
	mRenderCtx->ssao.blurShader = ResourceManager::instance().getShader("ssaoBlur");

	mLightSystem->configure(*mRenderCtx, eventBus);
	mSyncStateSystem->configure(*mRenderCtx, eventBus);
	mShadowSystem->configure(*mRenderCtx, eventBus);

	// Create framebuffers
	mSceneBuffer = std::make_unique<FrameBuffer>(SCR_WIDTH, SCR_HEIGHT);
#ifdef MSAA
	glEnable(GL_MULTISAMPLE);
	mIntermediateBuffer = std::make_unique<FrameBuffer>(mSceneBuffer->width(), mSceneBuffer->height());
# ifdef HDR
	mIntermediateBuffer->withTextureFP(GL_RGBA)
			.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureFPMultisampled(MULTISAMPLED_COUNT, GL_RGBA)
# else
	mIntermediateBuffer->withTexture(GL_RGBA)
			.withRenderBufferDepth(GL_DEPTH_COMPONENT24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureMultisampled(MULTISAMPLED_COUNT, GL_RGBA)
# endif
			.withRenderBufferDepthMultisampled(MULTISAMPLED_COUNT, GL_DEPTH_COMPONENT24)
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
		mRenderCtx->camera.ubo.binding);
	// Create render passes
	mRenderPasses.emplace_back(std::make_shared<FrustumCullingPass>());

	if (!mRenderQueue.deferredGroups.empty()) {
		mDeferredGeometryPass = std::make_shared<DeferredGeometryPass>();
		mDeferredLightingPass = std::make_shared<DeferredLightingPass>();
		mSSAOPass = std::make_shared<SSAOPass>();

		mRenderPasses.push_back(mDeferredGeometryPass);
		mRenderPasses.push_back(mSSAOPass);
		mRenderPasses.push_back(mDeferredLightingPass);
	}

	if (!mRenderQueue.opaqueGroups.empty() || !mRenderQueue.blendGroups.empty()) {
		mRenderPasses.emplace_back(std::make_shared<ForwardPass>());
	}

	if (!mRenderQueue.debugGroups.empty()) {
		mRenderPasses.emplace_back(std::make_shared<DebugPass>());
	}

	if (!mRenderQueue.opaqueInstancedGroups.empty() || !mRenderQueue.blendInstancedGroups.empty()) {
		mRenderPasses.emplace_back(std::make_shared<InstancedPass>());
	}

	mRenderPasses.emplace_back(std::make_shared<SkyboxPass>());
#ifdef MSAA
	mRenderPasses.emplace_back(std::make_shared<ResolvePass>());
	mRenderCtx->intermediateBuffer = mIntermediateBuffer.get();
#endif
	mRenderPasses.emplace_back(std::make_shared<PostProcessPass>());

	mRenderCtx->sceneBuffer = mSceneBuffer.get();
	mRenderCtx->camera.self = &camera;
	mRenderCtx->camera.frustum = &camera.frustum();
	mRenderCtx->camera.ubo.self = mCameraUBO.get();
	mRenderCtx->ssao.ubo.self = mSSAOPass ? mSSAOPass->ubo() : nullptr;
	mRenderCtx->light.ubo.self = mLightSystem->ubo();
	mRenderCtx->shadow.ubo.self = mShadowSystem->ubo();
	mRenderCtx->PBR.irradianceMap.self = ResourceManager::instance().getBuffer("irradianceMap");
	mRenderCtx->PBR.irradianceMap.textureSlot = PBR_IRRADIANCE_MAP_TEXTURE_SLOT;
	mRenderCtx->PBR.prefilterMap.self = ResourceManager::instance().getBuffer("prefilterMap");
	mRenderCtx->PBR.prefilterMap.textureSlot = PBR_PREFILTER_MAP_TEXTURE_SLOT;
	mRenderCtx->PBR.brdfLUT.self = ResourceManager::instance().getBuffer("brdfLUT");
	mRenderCtx->PBR.brdfLUT.textureSlot = PBR_BRDF_LUT_TEXTURE_SLOT;
	mRenderCtx->PBR.albedoTextureSlot = PBR_ALBEDO_TEXTURE_SLOT;
	mRenderCtx->PBR.normalTextureSlot = PBR_NORMAL_TEXTURE_SLOT;
	mRenderCtx->PBR.roughnessMetallicTextureSlot = PBR_RM_TEXTURE_SLOT;
	mRenderCtx->PBR.aoTextureSlot = PBR_AO_TEXTURE_SLOT;
	mRenderCtx->PBR.emissiveTextureSlot = PBR_EMISSIVE_TEXTURE_SLOT;
	mRenderCtx->PBR.heightTextureSlot = PBR_HEIGHT_TEXTURE_SLOT;

	// Set camera projection matrix
	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(ZOOM),
		static_cast<float>(mRenderCtx->screen.width) / static_cast<float>(mRenderCtx->screen.height),
		ZNEAR, ZFAR);

	const glm::mat4 invProjectionMat = glm::inverse(projectionMat);

	mRenderCtx->camera.ubo.self->bind();

	mRenderCtx->camera.ubo.self->setData(
		glm::value_ptr(projectionMat),
		sizeof(glm::mat4),
		sizeof(glm::mat4) + sizeof(glm::vec4));

	mRenderCtx->camera.ubo.self->setData(
		glm::value_ptr(invProjectionMat),
		sizeof(glm::mat4),
		2 * sizeof(glm::mat4) + sizeof(glm::vec4));

	mRenderCtx->camera.ubo.self->unbind();

	// Configure render passes
	for (const auto& pass: mRenderPasses) {
		pass->configure(*mRenderCtx, eventBus);
	}

	if (mDeferredGeometryPass) {
		mRenderCtx->gBuffer.self = mDeferredGeometryPass->gBuffer();
		mRenderCtx->ssao.buffer = mSSAOPass->blurFBO();
	}

	// Configure shaders
	for (const auto& [name,shader]: ResourceManager::instance().getShaders()) {
		mRenderCtx->camera.ubo.self->configure(
			shader->id(),
			mRenderCtx->camera.ubo.binding,
			mRenderCtx->camera.ubo.blockName);

		mRenderCtx->light.ubo.self->configure(
			shader->id(),
			mRenderCtx->light.ubo.binding,
			mRenderCtx->light.ubo.blockName);

		mRenderCtx->shadow.ubo.self->configure(
			shader->id(),
			mRenderCtx->shadow.ubo.binding,
			mRenderCtx->shadow.ubo.blockName);
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
	glViewport(
		0,
		0,
		static_cast<int32_t>(mRenderCtx->screen.width),
		static_cast<int32_t>(mRenderCtx->screen.height));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	mRenderCtx->camera.ubo.self->bind();
	mRenderCtx->camera.ubo.self->setData(&packed, sizeof(PackedView), 0);
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

	if (entity.hasComponent<SkyboxComponent>()) {
		const auto& material = matComponent.materials->at(0);

		mRenderQueue.material.flags.emplace_back(material.flags);
		mRenderQueue.material.alphaCutoffs.emplace_back(material.alphaCutoff);
		mRenderQueue.material.colors.emplace_back(0.0f);

		mRenderQueue.material.textures.push_back(material.textures[0].id);

		auto& meshes = entity.getComponent<MeshComponent>().meshes->at(0);
		mRenderQueue.mesh.vaos.push_back(meshes[0].vao().id());
		mRenderQueue.mesh.vertexCounts.push_back(meshes[0].vertices().size());
		mRenderQueue.mesh.indexCounts.push_back(meshes[0].indices().size());
		mRenderQueue.mesh.maxCounts.push_back(meshes[0].max());
		mRenderQueue.mesh.minCounts.push_back(meshes[0].min());

		std::vector<uint32_t> indices{meshIndex++};

		const MaterialBatch matBatch{
			materialIndex++,
			textureOffset++,
			1,
			ResourceManager::instance().getShader("skybox"),
			indices};
		const RenderGroup group{entity.id(), matBatch};
		mRenderQueue.skybox.push_back(group);
		return;
	}

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

		if (entity.hasComponent<InstanceComponent>()) {
			// Set shader
			if (material.flags & OPAQUE) {
				matBatch.shader = ResourceManager::instance().getShader("instancedOpaque");
			} else if (material.flags & BLEND) {
				matBatch.shader = ResourceManager::instance().getShader("instancedBlend");
			}

			const auto& instComponent = entity.getComponent<InstanceComponent>();
			InstanceGroup instance{entity.id(), *instComponent.transforms, matBatch};

			if (material.flags & OPAQUE) {
				mRenderQueue.opaqueInstancedGroups.push_back(instance);
			} else if (material.flags & BLEND) {
				mRenderQueue.blendInstancedGroups.push_back(instance);
			}

			continue;
		}
		// Set shader
		if (material.flags & OPAQUE) {
			if (material.flags & UNLIT) {
				matBatch.shader = ResourceManager::instance().getShader("unlit");
			} else {
				matBatch.shader = ResourceManager::instance().getShader("opaque");
			}
		} else if (material.flags & BLEND) {
			matBatch.shader = ResourceManager::instance().getShader("blend");
		}

		const RenderGroup group{entity.id(), matBatch};

		if (entity.hasComponent<DebugComponent>()) {
			mRenderQueue.debugGroups.push_back(group);
		}

		if (material.flags & CASTSHADOW) {
			mRenderQueue.shadowGroups.push_back(group);
		}

		if (material.flags & PBR) {
			mRenderQueue.deferredGroups.push_back(group);
		} else if (material.flags & OPAQUE) {
			mRenderQueue.opaqueGroups.push_back(group);
		} else if (material.flags & BLEND) {
			mRenderQueue.blendGroups.push_back(group);
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
