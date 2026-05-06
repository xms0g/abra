#include "directionalShadowPass.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "../../mesh/mesh.h"
#include "../../shader.h"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderableObject.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"

DirectionalShadowPass::DirectionalShadowPass(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureDepth(GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>("depth/depth.vert", "depth/depth.frag");
}

DirectionalShadowPass::~DirectionalShadowPass() = default;

uint32_t DirectionalShadowPass::depthTexture() const {
	return mDepthMap->texture();
}

glm::mat4 DirectionalShadowPass::lightSpaceMatrix() const {
	return mLightSpaceMatrix;
}

void DirectionalShadowPass::render(const RenderContext& ctx, const glm::vec4& direction) {
	const glm::vec3 lightPos = -glm::vec3(direction) * ctx.shadow.directional.height;
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
	glCullFace(GL_FRONT);
	glViewport(0, 0, static_cast<int32_t>(ctx.shadow.width), static_cast<int32_t>(ctx.shadow.height));

	for (const auto& [entityID, model, normal, matIdx, meshIdx, shader]: ctx.renderQueue->shadowedObjects) {
		RenderCommon::setupTransform(entityID, model, normal, *mDepthShader);

		const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

		RenderCommon::drawMesh(vao, vertexCount, indexCount);
	}
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}
