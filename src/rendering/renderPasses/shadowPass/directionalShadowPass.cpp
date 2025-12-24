#include "directionalShadowPass.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderableObject.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"

DirectionalShadowPass::DirectionalShadowPass(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureDepth()
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>("depth/depth.vert", "depth/depth.frag");
}

DirectionalShadowPass::~DirectionalShadowPass() = default;

uint32_t DirectionalShadowPass::getDepthTexture() const {
	return mDepthMap->texture();
}

glm::mat4 DirectionalShadowPass::getLightSpaceMatrix() const {
	return mLightSpaceMatrix;
}

void DirectionalShadowPass::render(const RenderContext& ctx, const glm::vec4& direction) {
	const glm::vec3 lightPos = -glm::vec3(direction) * 5.0f;
	const glm::mat4 lightProjection = glm::ortho(
		ctx.shadow.directional.left,
		ctx.shadow.directional.right,
		ctx.shadow.directional.bottom,
		ctx.shadow.directional.top,
		ctx.shadow.directional.nearPlane,
		ctx.shadow.directional.farPlane);
	const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	mLightSpaceMatrix = lightProjection * lightView;

	mDepthShader->activate();
	mDepthShader->setMat4("lightSpaceMatrix", mLightSpaceMatrix);

	// render scene from light's point of view
	mDepthMap->bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glViewport(0, 0, ctx.shadow.width, ctx.shadow.height);

	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->shadowingObjects) {
		RenderCommon::setupTransform(*entity, *mDepthShader);
		RenderCommon::drawMesh(*mesh);
	}
	mDepthMap->unbind();
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}
