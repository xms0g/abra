#include "perspectiveShadowPass.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderableObject.hpp"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"

PerspectiveShadowPass::PerspectiveShadowPass(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureDepthArray(ctx.shadow.perspective.maxLights, 24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>("depth/depth.vert", "depth/depth.frag");
}

PerspectiveShadowPass::~PerspectiveShadowPass() = default;

uint32_t PerspectiveShadowPass::depthTexture() const {
	return mDepthMap->texture();
}

FrameBuffer& PerspectiveShadowPass::depthMap() const {
	return *mDepthMap;
}

glm::mat4 PerspectiveShadowPass::lightSpaceMatrix(const int layer) const {
	return mLightSpaceMatrix[layer];
}

void PerspectiveShadowPass::render(const RenderContext& ctx, const glm::vec4& direction,
                                   const glm::vec4& position, const float fovy, const int layer) {
	const glm::mat4 lightProjection = glm::perspective(
		fovy,
		static_cast<float>(ctx.shadow.width) / static_cast<float>(ctx.shadow.height),
		ctx.shadow.perspective.nearPlane,
		ctx.shadow.perspective.farPlane);

	const auto dir = glm::vec3(direction);
	const auto pos = glm::vec3(position);
	const glm::mat4 lightView = glm::lookAt(pos, pos + dir, glm::vec3(0.0, 1.0, 0.0));
	mLightSpaceMatrix[layer] = lightProjection * lightView;

	mDepthShader->activate();
	mDepthShader->setMat4("lightSpaceMatrix", mLightSpaceMatrix[layer]);

	// render scene from light's point of view
	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->shadowingObjects) {
		RenderCommon::setupTransform(*entity, *mDepthShader);
		RenderCommon::drawMesh(*mesh);
	}
}
