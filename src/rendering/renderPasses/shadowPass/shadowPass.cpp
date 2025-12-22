#include "shadowPass.h"
#include "glad/glad.h"
#include "../../renderContext/renderContext.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../buffers/frameBuffer.h"
#include "../../../ECS/components/directionalLight.hpp"
#include "../../../ECS/components/pointLight.hpp"
#include "../../../ECS/components/spotLight.hpp"

ShadowPass::~ShadowPass() = default;

void ShadowPass::configure(const RenderContext& ctx) {
	mDirShadowPass = std::make_unique<DirectionalShadowPass>(ctx);
	mOmnidirShadowPass = std::make_unique<OmnidirectionalShadowPass>(ctx);
	mPerspectiveShadowPass = std::make_unique<PerspectiveShadowPass>(ctx);

	mShadowUBO = std::make_unique<UniformBuffer>(sizeof(ShadowData), 2);

	mShadowMaps = {
		mDirShadowPass->getDepthTexture(),
		mOmnidirShadowPass->getDepthTexture(),
		mPerspectiveShadowPass->getDepthTexture()
	};
}

void ShadowPass::execute(const RenderContext& ctx) {
	directionalShadowPass(ctx);
	omnidirectionalShadowPass(ctx);
	perspectiveShadowPass(ctx);

	mShadowUBO->bind();
	mShadowUBO->setData(&mShadowData, sizeof(ShadowData), 0);
	mShadowUBO->unbind();
}

const std::array<uint32_t, 3>& ShadowPass::getShadowMaps() const { return mShadowMaps; }

const UniformBuffer* ShadowPass::getShadowUBO() const { return mShadowUBO.get(); }

void ShadowPass::directionalShadowPass(const RenderContext& ctx) {
	for (const auto& light: *ctx.light.dirLights) {
		mDirShadowPass->render(ctx, light->direction);
		mShadowData.lightSpaceMatrix = mDirShadowPass->getLightSpaceMatrix();
	}
}

void ShadowPass::omnidirectionalShadowPass(const RenderContext& ctx) {
	mOmnidirShadowPass->getDepthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, ctx.shadowMap.width, ctx.shadowMap.height);

	const auto& lights = *ctx.light.pointLights;
	for (int i = 0; i < lights.size(); i++) {
		const auto& light = lights[i];
		if (!light->castShadow) continue;

		mOmnidirShadowPass->render(ctx, light->position, i);
		mShadowData.omniFarPlanes[i] = ctx.shadowMap.omnidirectional.farPlane;
	}

	mOmnidirShadowPass->getDepthMap().unbind();
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void ShadowPass::perspectiveShadowPass(const RenderContext& ctx) {
	mPerspectiveShadowPass->getDepthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glViewport(0, 0, ctx.shadowMap.width, ctx.shadowMap.height);

	const auto& lights = *ctx.light.spotLights;
	for (int i = 0; i < lights.size(); i++) {
		const auto& light = lights[i];
		if (!light->castShadow) continue;

		mPerspectiveShadowPass->getDepthMap().attachLayer(GL_DEPTH_ATTACHMENT, i);

		mPerspectiveShadowPass->render(ctx, light->direction, light->position, light->cutOff.y, i);
		mShadowData.persLightSpaceMatrix[i] = mPerspectiveShadowPass->getLightSpaceMatrix(i);
	}
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
	mPerspectiveShadowPass->getDepthMap().unbind();
}
