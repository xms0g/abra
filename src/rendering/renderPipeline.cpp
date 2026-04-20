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
#include "renderPasses/shadowPass/shadowPass.h"
#include "gui/backend.h"
#include "material/material.hpp"
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
	mRenderCtx->light.maxDirLights = MAX_DIRECTIONAL_LIGHTS;
	mRenderCtx->light.maxPointLights = MAX_POINT_LIGHTS;
	mRenderCtx->light.maxSpotLights = MAX_SPOT_LIGHTS;
	mRenderCtx->light.ubo.binding = LIGHT_UBO_BINDING;

	registry->addSystem<LightSystem>(*mRenderCtx);
	mLightSystem = &registry->getSystem<LightSystem>();

	opaque = std::make_unique<Shader>("object.vert", "opaque.frag");
	blend = std::make_unique<Shader>("object.vert", "blend.frag");
	unlit = std::make_unique<Shader>("unlit.vert", "unlit.frag");
	instancedOpaque = std::make_unique<Shader>("instanced.vert", "opaque.frag");
	instancedBlend = std::make_unique<Shader>("instanced.vert", "blend.frag");
	skybox = std::make_unique<Shader>("skybox.vert", "skybox.frag");

	GuiBackend::init(window, context, "#version 410");
}

RenderPipeline::~RenderPipeline() {
	GuiBackend::shutdown();
}

PostProcessPass& RenderPipeline::postProcess() const { return *mPostProcessPass; }

void RenderPipeline::configure(const Camera& camera) {
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
	mShadowPass = std::make_shared<ShadowPass>();
	mPostProcessPass = std::make_shared<PostProcessPass>();

	mRenderPasses.push_back(std::make_shared<FrustumCullingPass>());
	mRenderPasses.push_back(mShadowPass);

	if (!mRenderQueue.deferredGroups.empty()) {
		mDeferredGeometryPass = std::make_shared<DeferredGeometryPass>();
		mDeferredLightingPass = std::make_shared<DeferredLightingPass>();
		mSSAOPass = std::make_shared<SSAOPass>();

		mRenderPasses.push_back(mDeferredGeometryPass);
		mRenderPasses.push_back(mSSAOPass);
		mRenderPasses.push_back(mDeferredLightingPass);
	}

	if (!mRenderQueue.opaqueGroups.empty() || !mRenderQueue.blendGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<ForwardPass>());
	}

	if (!mRenderQueue.debugGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<DebugPass>());
	}

	if (!mRenderQueue.opaqueInstancedGroups.empty() || !mRenderQueue.blendInstancedGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<InstancedPass>());
	}

	mRenderPasses.push_back(std::make_shared<SkyboxPass>());
#ifdef MSAA
	mRenderPasses.push_back(std::make_shared<ResolvePass>());
	mRenderCtx->intermediateBuffer = mIntermediateBuffer.get();
#endif
	mRenderPasses.push_back(mPostProcessPass);

	mRenderCtx->sceneBuffer = mSceneBuffer.get();
	mRenderCtx->camera.self = &camera;
	mRenderCtx->camera.frustum = &camera.frustum();
	mRenderCtx->camera.ubo.self = mCameraUBO.get();
	mRenderCtx->camera.ubo.blockName = CAMERA_UBO_BLOCK_NAME;
	mRenderCtx->ssao.ubo.self = mSSAOPass ? mSSAOPass->ubo() : nullptr;
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
	mRenderCtx->light.ubo.self = &mLightSystem->ubo();
	mRenderCtx->light.ubo.blockName = LIGHT_UBO_BLOCK_NAME;
	mRenderCtx->light.dirLights = &mLightSystem->dirLights();
	mRenderCtx->light.pointLights = &mLightSystem->pointLights();
	mRenderCtx->light.spotLights = &mLightSystem->spotLights();
	mRenderCtx->shadow.ubo.self = mShadowPass->ubo();
	mRenderCtx->shadow.ubo.binding = SHADOW_UBO_BINDING;
	mRenderCtx->shadow.ubo.blockName = SHADOW_UBO_BLOCK_NAME;
	mRenderCtx->shadow.textures = &mShadowPass->shadowMaps();
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
	mRenderCtx->PBR.envMap.size = PBR_ENVMAP_SIZE;
	mRenderCtx->PBR.irradianceMap.size = PBR_IRRADIANCE_MAP_SIZE;
	mRenderCtx->PBR.irradianceMap.textureSlot = PBR_IRRADIANCE_MAP_TEXTURE_SLOT;
	mRenderCtx->PBR.prefilterMap.size = PBR_PREFILTER_MAP_SIZE;
	mRenderCtx->PBR.prefilterMap.textureSlot = PBR_PREFILTER_MAP_TEXTURE_SLOT;
	mRenderCtx->PBR.brdfLUT.size = PBR_BRDF_LUT_SIZE;
	mRenderCtx->PBR.brdfLUT.textureSlot = PBR_BRDF_LUT_TEXTURE_SLOT;
	mRenderCtx->PBR.HDRTexture = PBR_HDR_TEXTURE;
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
		pass->configure(*mRenderCtx);
	}

	if (mDeferredGeometryPass) {
		mRenderCtx->gBuffer.self = mDeferredGeometryPass->gBuffer();
		mRenderCtx->ssao.buffer = mSSAOPass->blurFBO();
	}

	// Configure shaders
	const std::vector<Shader*> shaders = {
		opaque.get(), blend.get(), unlit.get(),
		instancedOpaque.get(), instancedBlend.get(), skybox.get()
	};

	for (const auto& shader: shaders) {
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

		shader->activate();
		shader->setInt("material.texture_albedo", PBR_ALBEDO_TEXTURE_SLOT);
		shader->setInt("shadowMap", mRenderCtx->shadow.textureSlot);
		shader->setInt("shadowCubemap", mRenderCtx->shadow.textureSlot + 1);
		shader->setInt("persShadowMap", mRenderCtx->shadow.textureSlot + 2);
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

	mLightSystem->update(*mRenderCtx);

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

	struct PackedView {
		glm::mat4 view;
		glm::vec4 viewPos;
	};

	const PackedView packed = {
		mRenderCtx->camera.self->viewMatrix(),
		glm::vec4(mRenderCtx->camera.self->position(), 1.0)
	};

	mRenderCtx->camera.ubo.self->bind();
	mRenderCtx->camera.ubo.self->setData(&packed, sizeof(PackedView), 0);
	mRenderCtx->camera.ubo.self->unbind();
}

void RenderPipeline::batchEntity(const Entity& entity) {
	const auto& matComponent = entity.getComponent<MaterialComponent>();

	const EntityCore entityCore{
		&entity.getComponent<DebugComponent>(),
		&entity.getComponent<TransformComponent>(),
		&entity.getComponent<MaterialComponent>(),
		!entity.hasComponent<SkyboxComponent>()
			? entity.getComponent<BoundingVolumeComponent>().bv->center()
			: glm::vec3(0.0f),
		!entity.hasComponent<SkyboxComponent>()
			? entity.getComponent<BoundingVolumeComponent>().bv->extents()
			: glm::vec3(0.0f),
	};

	if (entity.hasComponent<SkyboxComponent>()) {
		const auto& material = matComponent.materials->at(0);
		auto& meshes = entity.getComponent<MeshComponent>().meshes->at(0);
		const MaterialBatch matBatch{&material, skybox.get(), &meshes};
		const RenderGroup group{entityCore, matBatch};
		mRenderQueue.skybox.push_back(group);
		return;
	}

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		const auto& material = matComponent.materials->at(matID);
		MaterialBatch matBatch{&material, nullptr, &meshes};

		if (matComponent.renderFlag & INSTANCED_PASS) {
			// Set shader
			if (material.flags & OPAQUE) {
				matBatch.shader = instancedOpaque.get();
			} else if (material.flags & BLEND) {
				matBatch.shader = instancedBlend.get();
			}

			const auto& instComponent = entity.getComponent<InstanceComponent>();
			InstanceGroup instance{entityCore, instComponent.transforms, matBatch};

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
				matBatch.shader = unlit.get();
			} else {
				matBatch.shader = opaque.get();
			}
		} else if (material.flags & BLEND) {
			matBatch.shader = blend.get();
		}

		const RenderGroup group{entityCore, matBatch};

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
			[&camPos, &transparent](const RenderGroup& a, const RenderGroup& b) {
				const float da = glm::length2(camPos - a.entity.transform->position);
				const float db = glm::length2(camPos - b.entity.transform->position);

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
