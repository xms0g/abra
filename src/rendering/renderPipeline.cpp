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
#include "renderPasses/forwardOpaquePass.h"
#include "renderPasses/pbrPass.h"
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
#include "../ECS/components/bv.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/instance.hpp"
#include "../ECS/components/skybox.hpp"

RenderPipeline::RenderPipeline(Registry* registry) {
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
	cutout = std::make_unique<Shader>("object.vert", "cutout.frag");
	blend = std::make_unique<Shader>("object.vert", "blend.frag");
	unlit = std::make_unique<Shader>("unlit.vert", "unlit.frag");
	pbr = std::make_unique<Shader>("pbr/pbr.vert", "pbr/pbr.frag");
	instancedOpaque = std::make_unique<Shader>("instanced.vert", "opaque.frag");
	instancedCutout = std::make_unique<Shader>("instanced.vert", "cutout.frag");
	instancedBlend = std::make_unique<Shader>("instanced.vert", "blend.frag");
	skybox = std::make_unique<Shader>("skybox.vert", "skybox.frag");
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
	mIntermediateBuffer->withTextureFP(16, true)
			.withRenderBufferDepth(24)
			.checkStatus();
	mIntermediateBuffer->unbind();

	mSceneBuffer->bind();
	mSceneBuffer->withTextureFPMultisampled(MULTISAMPLED_COUNT, 16, true)
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
	mSceneBuffer->withTextureFP(16, true)
# else
	mSceneBuffer->withTexture()
# endif
			.withTextureDepth(24, false)
#endif
			.checkStatus();
	mSceneBuffer->unbind();

	// Create camera buffer
	mCameraUBO = std::make_unique<UniformBuffer>(3 * sizeof(glm::mat4) + sizeof(glm::vec4),
	                                             mRenderCtx->camera.ubo.binding);
	// Create render passes
	mShadowPass = std::make_shared<ShadowPass>();
	mPostProcessPass = std::make_shared<PostProcessPass>();

	mRenderPasses.push_back(std::make_shared<FrustumCullingPass>());
	mRenderPasses.push_back(mShadowPass);
	mRenderPasses.push_back(std::make_shared<BeginScenePass>());

	if (!mRenderQueue.deferredGroups.empty()) {
		mDeferredGeometryPass = std::make_shared<DeferredGeometryPass>();
		mDeferredLightingPass = std::make_shared<DeferredLightingPass>();
		mSSAOPass = std::make_shared<SSAOPass>();

		mRenderPasses.push_back(mDeferredGeometryPass);
		mRenderPasses.push_back(mSSAOPass);
		mRenderPasses.push_back(mDeferredLightingPass);
	}

	if (!mRenderQueue.forwardOpaqueGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<ForwardOpaquePass>());
	}

	if (!mRenderQueue.pbrGroups.empty()) {
		mRenderPasses.push_back(std::make_shared<PBRPass>());
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
	mRenderCtx->PBR.HDRTexture = PBR_HDR_TEXTURE;

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
		mRenderCtx->gBuffer = mDeferredGeometryPass->gBuffer();
		mRenderCtx->ssao.buffer = mSSAOPass->blurFBO();
	}

	// Configure shaders
	const std::vector<Shader*> shaders = {
		opaque.get(), cutout.get(), blend.get(),
		unlit.get(), pbr.get(), instancedOpaque.get(),
		instancedCutout.get(), instancedBlend.get(), skybox.get()
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
	}
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
	glViewport(
		0,
		0,
		static_cast<int32_t>(mRenderCtx->screen.width),
		static_cast<int32_t>(mRenderCtx->screen.height));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPipeline::refreshCameraData() const {
	mRenderCtx->camera.skyView = glm::mat4(glm::mat3(mRenderCtx->camera.self->viewMatrix()));

	auto view = mRenderCtx->camera.self->viewMatrix();
	auto viewPos = glm::vec4(mRenderCtx->camera.self->position(), 1.0);
	mRenderCtx->camera.ubo.self->bind();
	mRenderCtx->camera.ubo.self->setData(glm::value_ptr(view), sizeof(glm::mat4), 0);
	mRenderCtx->camera.ubo.self->setData(glm::value_ptr(viewPos), sizeof(glm::vec4), sizeof(glm::mat4));
	mRenderCtx->camera.ubo.self->unbind();
}

void RenderPipeline::batchEntity(const Entity& entity) {
	const auto& matc = entity.getComponent<MaterialComponent>();
	const EntityData eData{
		&entity.getComponent<DebugComponent>(),
		&entity.getComponent<TransformComponent>(),
		&entity.getComponent<MaterialComponent>(),
		entity.getComponent<BoundingVolumeComponent>().bv.get()
	};

	if (entity.hasComponent<SkyboxComponent>()) {
		const auto& material = matc.materials->at(0);
		auto& meshes = entity.getComponent<MeshComponent>().meshes->at(0);
		const MaterialBatch matb{&material, skybox.get(), &meshes};
		const RenderGroup group{eData, matb};
		mRenderQueue.skybox.push_back(group);
		return;
	}

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		const auto& material = matc.materials->at(matID);
		MaterialBatch matb{&material, nullptr, &meshes};

		if (matc.renderFlag & INSTANCED_PASS) {
			// Set shader
			if (material.flag & OPAQUE) {
				matb.shader = instancedOpaque.get();
			} else if (material.flag & CUTOUT) {
				matb.shader = instancedCutout.get();
			} else if (material.flag & BLEND) {
				matb.shader = instancedBlend.get();
			}

			const auto& ic = entity.getComponent<InstanceComponent>();
			InstanceGroup instance{eData, ic.transforms, matb};

			if (material.flag & OPAQUE) {
				mRenderQueue.opaqueInstancedGroups.push_back(instance);
			} else if (material.flag & CUTOUT) {
				mRenderQueue.cutoutInstancedGroups.push_back(instance);
			} else if (material.flag & BLEND) {
				mRenderQueue.blendInstancedGroups.push_back(instance);
			}

			continue;
		}
		// Set shader
		if (material.flag & OPAQUE) {
			if (material.flag & UNLIT) {
				matb.shader = unlit.get();
			} else {
				matb.shader = opaque.get();
			}
		} else if (material.flag & PBR) {
			matb.shader = pbr.get();
		} else if (material.flag & CUTOUT) {
			matb.shader = cutout.get();
		} else if (material.flag & BLEND) {
			matb.shader = blend.get();
		}

		const RenderGroup group{eData, matb};

		if (entity.hasComponent<DebugComponent>()) {
			mRenderQueue.debugGroups.push_back(group);
		}

		if (material.flag & CASTSHADOW) {
			mRenderQueue.shadowGroups.push_back(group);
		}

		if (material.flag & OPAQUE) {
			if (matc.renderFlag & FORWARD_PASS) {
				mRenderQueue.forwardOpaqueGroups.push_back(group);
			} else {
				mRenderQueue.deferredGroups.push_back(group);
			}
		} else if (material.flag & CUTOUT) {
			mRenderQueue.forwardOpaqueGroups.push_back(group);
		} else if (material.flag & BLEND) {
			mRenderQueue.blendGroups.push_back(group);
		} else if (material.flag & PBR) {
			mRenderQueue.pbrGroups.push_back(group);
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
				const float da = glm::length2(camPos - a.eData.transform->position);
				const float db = glm::length2(camPos - b.eData.transform->position);

				if (transparent)
					return da > db;
				return da < db;
			}
		);
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
