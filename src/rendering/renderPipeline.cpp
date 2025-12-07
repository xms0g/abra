#include "renderPipeline.h"
#include <SDL.h>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "shader.h"
#include "lightSystem.h"
#include "buffers/frameBuffer.h"
#include "buffers/uniformBuffer.h"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderFlags.hpp"
#include "renderContext/renderGroup.hpp"
#include "renderContext/instanceGroup.hpp"
#include "renderPasses/IRenderPass.hpp"
#include "renderPasses/deferredGeometryPass.h"
#include "renderPasses/deferredLightingPass.h"
#include "renderPasses/debugPass.h"
#include "renderPasses/forwardOpaquePass.h"
#include "renderPasses/blendInstancedPass.h"
#include "renderPasses/blendPass.h"
#include "renderPasses/opaqueInstancedPass.h"
#include "renderPasses/beginScenePass.h"
#include "renderPasses/frustumCullingPass.h"
#include "renderPasses/skyboxPass.h"
#include "renderPasses/resolvePass.h"
#include "renderPasses/postProcess/postProcessPass.h"
#include "renderPasses/shadowPass/shadowPass.h"
#include "material/material.hpp"
#include "../config/config.hpp"
#include "../ECS/registry.h"
#include "../core/camera.h"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/shader.hpp"
#include "../ECS/components/instance.hpp"
#include "../ECS/components/skybox.hpp"

RenderPipeline::RenderPipeline(Registry* registry) {
	RequireComponent<MeshComponent>();
	RequireComponent<ShaderComponent>();
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
	mRenderCtx->light.maxDirLights = MAX_DIRECTIONAL_LIGHTS;
	mRenderCtx->light.maxPointLights = MAX_POINT_LIGHTS;
	mRenderCtx->light.maxSpotLights = MAX_SPOT_LIGHTS;

	registry->addSystem<LightSystem>(*mRenderCtx);
	mLightSystem = &registry->getSystem<LightSystem>();
}

RenderPipeline::~RenderPipeline() = default;

PostProcessPass& RenderPipeline::postProcess() const { return *mPostProcessPass; }

void RenderPipeline::configure(const Camera& camera) {
	// Create framebuffers
	mSceneBuffer = std::make_unique<FrameBuffer>(SCR_WIDTH, SCR_HEIGHT);
#ifdef MSAA
	glEnable(GL_MULTISAMPLE);
	mIntermediateBuffer = std::make_unique<FrameBuffer>(mSceneBuffer->width(), mSceneBuffer->height());
# ifdef HDR
	mIntermediateBuffer->withTextureFP(true, 16)
			.withRenderBufferDepth(24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureFPMultisampled(true, 16, MULTISAMPLED_COUNT)
# else
	mIntermediateBuffer->withTexture()
			.withRenderBufferDepth(24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureMultisampled(MULTISAMPLED_COUNT)
# endif
			.withRenderBufferDepthMultisampled(MULTISAMPLED_COUNT, 24)
#else
# ifdef HDR
	mSceneBuffer->withTextureFP(true, 16)
# else
	mSceneBuffer->withTexture()
# endif
			.withRenderBufferDepth(24)
#endif
			.checkStatus();
	mSceneBuffer->unbind();

	// Create camera buffer
	mCameraUBO = std::make_unique<UniformBuffer>(2 * sizeof(glm::mat4) + sizeof(glm::vec4), 0);

	// Create render passes
	mShadowPass = std::make_shared<ShadowPass>();
	mPostProcessPass = std::make_shared<PostProcessPass>();

	mRenderPasses.push_back(std::make_shared<FrustumCullingPass>());
	mRenderPasses.push_back(mShadowPass);
	mRenderPasses.push_back(std::make_shared<BeginScenePass>());

	if (!mRenderQueue.deferredGroups.empty()) {
		mDeferredGeometryPass = std::make_shared<DeferredGeometryPass>();
		mDeferredLightingPass = std::make_shared<DeferredLightingPass>();

		mRenderPasses.push_back(mDeferredGeometryPass);
		mRenderPasses.push_back(mDeferredLightingPass);
	}

	if (!mRenderQueue.forwardOpaqueGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<ForwardOpaquePass>());
	}

	if (!mRenderQueue.debugGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<DebugPass>());
	}

	if (!mRenderQueue.opaqueInstancedGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<OpaqueInstancedPass>());
	}

	if (!mRenderQueue.blendGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<BlendPass>());
	}

	if (!mRenderQueue.blendInstancedGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<BlendInstancedPass>());
	}

	mRenderPasses.push_back(std::make_shared<SkyboxPass>());
#ifdef MSAA
	mRenderPasses.push_back(std::make_shared<ResolvePass>());
#endif
	mRenderPasses.push_back(mPostProcessPass);

	mRenderCtx->renderQueue = &mRenderQueue;
	mRenderCtx->sceneBuffer = mSceneBuffer.get();
	mRenderCtx->intermediateBuffer = mIntermediateBuffer.get();
	mRenderCtx->light.ubo = &mLightSystem->getLightUBO();
	mRenderCtx->light.dirLights = &mLightSystem->getDirLights();
	mRenderCtx->light.pointLights = &mLightSystem->getPointLights();
	mRenderCtx->light.spotLights = &mLightSystem->getSpotLights();
	mRenderCtx->camera.self = &camera;
	mRenderCtx->camera.ubo = mCameraUBO.get();
	mRenderCtx->screen.width = SCR_WIDTH;
	mRenderCtx->screen.height = SCR_HEIGHT;
	mRenderCtx->shadowMap.textureSlot = SHADOWMAP_TEXTURE_SLOT;
	mRenderCtx->shadowMap.width = SHADOWMAP_WIDTH;
	mRenderCtx->shadowMap.height = SHADOWMAP_HEIGHT;
	mRenderCtx->shadowMap.directional.maxLights = MAX_DIRECTIONAL_LIGHTS;
	mRenderCtx->shadowMap.directional.nearPlane = SHADOW_DIRECTIONAL_NEAR;
	mRenderCtx->shadowMap.directional.farPlane = SHADOW_DIRECTIONAL_FAR;
	mRenderCtx->shadowMap.directional.left = SHADOW_DIRECTIONAL_LEFT;
	mRenderCtx->shadowMap.directional.right = SHADOW_DIRECTIONAL_RIGHT;
	mRenderCtx->shadowMap.directional.bottom = SHADOW_DIRECTIONAL_BOTTOM;
	mRenderCtx->shadowMap.directional.top = SHADOW_DIRECTIONAL_TOP;
	mRenderCtx->shadowMap.omnidirectional.maxLights = MAX_POINT_LIGHTS;
	mRenderCtx->shadowMap.omnidirectional.nearPlane = SHADOW_OMNIDIRECTIONAL_NEAR;
	mRenderCtx->shadowMap.omnidirectional.farPlane = SHADOW_OMNIDIRECTIONAL_FAR;
	mRenderCtx->shadowMap.omnidirectional.fovy = SHADOW_OMNIDIRECTIONAL_FOVY;
	mRenderCtx->shadowMap.perspective.maxLights = MAX_SPOT_LIGHTS;
	mRenderCtx->shadowMap.perspective.nearPlane = SHADOW_PERSPECTIVE_NEAR;
	mRenderCtx->shadowMap.perspective.farPlane = SHADOW_PERSPECTIVE_FAR;
	mRenderCtx->shadowMap.ubo = mShadowPass->getShadowUBO();
	mRenderCtx->shadowMap.textures = &mShadowPass->getShadowMaps();

	// Configure render passes
	for (const auto& pass: mRenderPasses) {
		pass->configure(*mRenderCtx);
	}

	if (!mRenderQueue.deferredGroups.empty()) {
		mDeferredLightingPass->configureInput(mDeferredGeometryPass->getGBuffer());
	}

	// Configure entities
	for (const auto& entity: getSystemEntities()) {
		const auto& shader = entity.getComponent<ShaderComponent>().shader;

		mRenderCtx->camera.ubo->configure(shader->ID(), 0, "CameraBlock");
		mRenderCtx->light.ubo->configure(shader->ID(), 1, "LightBlock");
		mRenderCtx->shadowMap.ubo->configure(shader->ID(), 2, "ShadowBlock");
	}

	const glm::mat4 projectionMat = glm::perspective(
		glm::radians(mRenderCtx->camera.self->zoom()),
		static_cast<float>(mRenderCtx->screen.width) / static_cast<float>(mRenderCtx->screen.height),
		ZNEAR, ZFAR);

	mRenderCtx->camera.ubo->bind();
	mRenderCtx->camera.ubo->setData(glm::value_ptr(projectionMat), sizeof(glm::mat4), sizeof(glm::mat4));
	mRenderCtx->camera.ubo->unbind();
}

void RenderPipeline::batchEntities() {
	for (const auto& entity: getSystemEntities()) {
		batchEntity(entity);
	}
}

void RenderPipeline::render() {
	refreshCameraData();
	sortEntities();

	mLightSystem->update(*mRenderCtx);

	for (const auto& pass: mRenderPasses) {
		pass->execute(*mRenderCtx);
	}

	mRenderCtx->sceneBuffer = mSceneBuffer.get();
	glViewport(0, 0, static_cast<int32_t>(SCR_WIDTH), static_cast<int32_t>(SCR_HEIGHT));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPipeline::refreshCameraData() const {
	const auto skyView = glm::mat4(glm::mat3(mRenderCtx->camera.self->viewMatrix()));
	mRenderCtx->camera.skyView = skyView;
	mRenderCtx->camera.frustum = &mRenderCtx->camera.self->frustum();

	auto view = mRenderCtx->camera.self->viewMatrix();
	auto viewPos = glm::vec4(mRenderCtx->camera.self->position(), 1.0);
	mRenderCtx->camera.ubo->bind();
	mRenderCtx->camera.ubo->setData(glm::value_ptr(view), sizeof(glm::mat4), 0);
	mRenderCtx->camera.ubo->setData(glm::value_ptr(viewPos), sizeof(glm::vec4), 2 * sizeof(glm::mat4));
	mRenderCtx->camera.ubo->unbind();
}

void RenderPipeline::batchEntity(const Entity& entity) {
	if (entity.hasComponent<SkyboxComponent>()) {
		mRenderQueue.skyboxEntity = &entity;
		return;
	}

	const auto& matc = entity.getComponent<MaterialComponent>();
	const auto& shader = *entity.getComponent<ShaderComponent>().shader;
	const auto& materials = entity.getComponent<MaterialComponent>().materials;

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		const auto& material = materials->at(matID);
		std::vector<MaterialBatch> matBatch;
		matBatch.emplace_back(&material, &shader, &meshes);

		if (matc.flag & Instanced) {
			const auto& ic = entity.getComponent<InstanceComponent>();
			InstanceGroup instance{&entity, ic.transforms, matBatch};

			if (material.flag & Blend) {
				mRenderQueue.blendInstancedGroups.push_back(instance);
			} else {
				mRenderQueue.opaqueInstancedGroups.push_back(instance);
			}
			continue;
		}

		RenderGroup group{&entity, matBatch};

		if (entity.hasComponent<DebugComponent>()) {
			mRenderQueue.debugGroups.push_back(group);
		}

		if (material.flag & CastShadow) {
			mRenderQueue.shadowCasterGroups.push_back(group);
		}

		if (material.flag & Opaque) {
			if (matc.flag & Forward) {
				mRenderQueue.forwardOpaqueGroups.push_back(group);
			} else {
				mRenderQueue.deferredGroups.push_back(group);
			}
		} else if (material.flag & Cutout) {
			mRenderQueue.forwardOpaqueGroups.push_back(group);
		} else if (material.flag & Blend) {
			mRenderQueue.blendGroups.push_back(group);
		}
	}
}

void RenderPipeline::sortEntities() {
	const glm::vec3& camPos = mRenderCtx->camera.self->position();

	auto sortBatches = [&](auto& batch, bool transparent) {
		std::sort(
			batch.begin(),
			batch.end(),
			[&camPos, &transparent](const RenderGroup& a, const RenderGroup& b) {
				const float da = glm::length2(camPos - a.entity->getComponent<TransformComponent>().position);
				const float db = glm::length2(camPos - b.entity->getComponent<TransformComponent>().position);

				if (transparent)
					return da > db;
				return da < db;
			});
	};

	// Sort opaque objects front to back
	sortBatches(mRenderQueue.deferredGroups, false);
	sortBatches(mRenderQueue.forwardOpaqueGroups, false);
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
