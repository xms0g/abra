#include "omnidirectionalShadowPass.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderableObject.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../renderContext/renderContext.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"

OmnidirectionalShadowPass::OmnidirectionalShadowPass(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadowMap.width, ctx.shadowMap.height);
	mDepthMap->withTextureCubemapArrayDepth(ctx.shadowMap.omnidirectional.maxLights)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>("depth/depthCubemap.vert",
	                                        "depth/depthCubemap.frag",
	                                        "depth/depthCubemap.geom");
}

OmnidirectionalShadowPass::~OmnidirectionalShadowPass() = default;

uint32_t OmnidirectionalShadowPass::getDepthTexture() const {
	return mDepthMap->texture();
}

FrameBuffer& OmnidirectionalShadowPass::getDepthMap() const {
	return *mDepthMap;
}

void OmnidirectionalShadowPass::render(const RenderContext& ctx, const glm::vec4& position,
                                       const int layer) const {
	const glm::mat4 shadowProj = glm::perspective(
		glm::radians(ctx.shadowMap.omnidirectional.fovy),
		static_cast<float>(ctx.shadowMap.width) / static_cast<float>(ctx.shadowMap.height),
		ctx.shadowMap.omnidirectional.nearPlane,
		ctx.shadowMap.omnidirectional.farPlane);

	const auto pos = glm::vec3(position);
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	mDepthShader->activate();
	for (unsigned int i = 0; i < 6; ++i)
		mDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

	mDepthShader->setFloat("omniFarPlane", ctx.shadowMap.omnidirectional.farPlane);
	mDepthShader->setVec3("lightPos", position);
	mDepthShader->setInt("cubeIndex", layer);

	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->shadowingObjects) {
		RenderCommon::setupTransform(*entity, *mDepthShader);
		RenderCommon::drawMesh(*mesh);
	}
}
