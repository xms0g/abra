#include "shadowPass.h"
#include "glad/glad.h"
#include "directionalShadowPass.h"
#include "omnidirectionalShadowPass.h"
#include "perspectiveShadowPass.h"
#include "../../renderContext/renderContext.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../buffers/frameBuffer.h"
#include "../../../ECS/components/directionalLight.hpp"
#include "../../../ECS/components/pointLight.hpp"
#include "../../../ECS/components/spotLight.hpp"
#include "../../../config/config.hpp"

struct ShadowPass::ShadowPassImpl {
	std::unique_ptr<DirectionalShadowPass> dirShadowPass;
	std::unique_ptr<OmnidirectionalShadowPass> omnidirShadowPass;
	std::unique_ptr<PerspectiveShadowPass> perspectiveShadowPass;
};

struct alignas(16) ShadowData {
	glm::mat4 lightSpaceMatrix;
	glm::mat4 persLightSpaceMatrix[MAX_SPOT_LIGHTS];
	glm::vec4 omniFarPlanes;
};
static ShadowData shadowData{};

ShadowPass::ShadowPass(): mImpl(std::make_unique<ShadowPassImpl>()) {
}

ShadowPass::~ShadowPass() = default;

const UniformBuffer* ShadowPass::ubo() const {
	return mUBO.get();
}

const std::array<uint32_t, 3>& ShadowPass::shadowMaps() const {
	return mShadowMaps;
}

void ShadowPass::configure(const RenderContext& ctx) {
	mImpl->dirShadowPass = std::make_unique<DirectionalShadowPass>(ctx);
	mImpl->omnidirShadowPass = std::make_unique<OmnidirectionalShadowPass>(ctx);
	mImpl->perspectiveShadowPass = std::make_unique<PerspectiveShadowPass>(ctx);

	mUBO = std::make_unique<UniformBuffer>(sizeof(ShadowData), ctx.shadow.ubo.binding);

	mShadowMaps = {
		mImpl->dirShadowPass->depthTexture(),
		mImpl->omnidirShadowPass->depthTexture(),
		mImpl->perspectiveShadowPass->depthTexture()
	};
}

void ShadowPass::execute(const RenderContext& ctx) {
	directionalShadowPass(ctx);
	omnidirectionalShadowPass(ctx);
	perspectiveShadowPass(ctx);

	mUBO->bind();
	mUBO->setData(&shadowData, sizeof(ShadowData), 0);
	mUBO->unbind();
}

void ShadowPass::directionalShadowPass(const RenderContext& ctx) const {
	for (const auto& light: *ctx.light.dirLights) {
		mImpl->dirShadowPass->render(ctx, light->direction);
		shadowData.lightSpaceMatrix = mImpl->dirShadowPass->lightSpaceMatrix();
	}
}

void ShadowPass::omnidirectionalShadowPass(const RenderContext& ctx) const {
	mImpl->omnidirShadowPass->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, ctx.shadow.width, ctx.shadow.height);

	const auto& lights = *ctx.light.pointLights;
	for (int i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light->castShadow) continue;

		mImpl->omnidirShadowPass->render(ctx, light->position, i);
		shadowData.omniFarPlanes[i] = ctx.shadow.omnidirectional.farPlane;
	}

	mImpl->omnidirShadowPass->depthMap().unbind();
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void ShadowPass::perspectiveShadowPass(const RenderContext& ctx) const {
	mImpl->perspectiveShadowPass->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glViewport(0, 0, ctx.shadow.width, ctx.shadow.height);

	const auto& lights = *ctx.light.spotLights;
	for (int i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light->castShadow) continue;

		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			mImpl->perspectiveShadowPass->depthMap().texture(),
			0,
			i);

		mImpl->perspectiveShadowPass->render(ctx, light->direction, light->position, light->cutOff.y, i);
		shadowData.persLightSpaceMatrix[i] = mImpl->perspectiveShadowPass->lightSpaceMatrix(i);
	}
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
	mImpl->perspectiveShadowPass->depthMap().unbind();
}
