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

struct alignas(16) ShadowData {
	glm::mat4 lightSpaceMatrix;
	glm::mat4 persLightSpaceMatrix[MAX_SPOT_LIGHTS];
	glm::vec4 omniFarPlanes;
};
static ShadowData shadowData{};

ShadowPass::ShadowPass() = default;

ShadowPass::~ShadowPass() = default;

const UniformBuffer* ShadowPass::ubo() const {
	return mUBO.get();
}

const std::array<uint32_t, 3>& ShadowPass::shadowMaps() const {
	return mShadowMaps;
}

void ShadowPass::configure(const RenderContext& ctx) {
	dirShadowPass = std::make_unique<DirectionalShadowPass>(ctx);
	omnidirShadowPass = std::make_unique<OmnidirectionalShadowPass>(ctx);
	persShadowPass = std::make_unique<PerspectiveShadowPass>(ctx);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(ShadowData), ctx.shadow.ubo.binding);

	mShadowMaps = {
		dirShadowPass->depthTexture(),
		omnidirShadowPass->depthTexture(),
		persShadowPass->depthTexture()
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
		dirShadowPass->render(ctx, light->direction);
		shadowData.lightSpaceMatrix = dirShadowPass->lightSpaceMatrix();
	}
}

void ShadowPass::omnidirectionalShadowPass(const RenderContext& ctx) const {
	const auto& lights = *ctx.light.pointLights;
	if (lights.empty()) return;

	omnidirShadowPass->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, static_cast<int32_t>(ctx.shadow.width), static_cast<int32_t>(ctx.shadow.height));

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light->castShadow)
			continue;

		omnidirShadowPass->render(ctx, light->position, i);
		shadowData.omniFarPlanes[i] = ctx.shadow.omnidirectional.farPlane;
	}

	omnidirShadowPass->depthMap().unbind();
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void ShadowPass::perspectiveShadowPass(const RenderContext& ctx) const {
	const auto& lights = *ctx.light.spotLights;
	if (lights.empty()) return;

	persShadowPass->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glViewport(0, 0, static_cast<int32_t>(ctx.shadow.width), static_cast<int32_t>(ctx.shadow.height));

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light->castShadow)
			continue;

		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			persShadowPass->depthMap().texture(),
			0,
			i);

		persShadowPass->render(ctx, light->direction, light->position, light->cutOff.y, i);
		shadowData.persLightSpaceMatrix[i] = persShadowPass->lightSpaceMatrix(i);
	}
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
	persShadowPass->depthMap().unbind();
}
