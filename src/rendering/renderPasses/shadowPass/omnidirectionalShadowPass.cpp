#include "omnidirectionalShadowPass.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderableObject.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../renderContext/renderContext.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"
#include "../../mesh/mesh.h"
#include "../../mesh/vertex.hpp"

OmnidirectionalShadowPass::OmnidirectionalShadowPass(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureCubemapDepthArray(ctx.shadow.omnidirectional.maxLights, GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>(
		"depth/depthCubemap.vert",
		"depth/depthCubemap.frag",
		"depth/depthCubemap.geom");
}

OmnidirectionalShadowPass::~OmnidirectionalShadowPass() = default;

uint32_t OmnidirectionalShadowPass::depthTexture() const {
	return mDepthMap->texture();
}

FrameBuffer& OmnidirectionalShadowPass::depthMap() const {
	return *mDepthMap;
}

void OmnidirectionalShadowPass::render(
	const RenderContext& ctx,
	const glm::vec4& position,
	const int32_t layer) const {
	const glm::mat4 shadowProj = glm::perspective(
		glm::radians(ctx.shadow.omnidirectional.fovy),
		static_cast<float>(ctx.shadow.width) / static_cast<float>(ctx.shadow.height),
		ctx.shadow.omnidirectional.nearPlane,
		ctx.shadow.omnidirectional.farPlane);

	const auto pos = glm::vec3(position);
	std::vector<glm::mat4> shadowTransforms;
	for (const auto& [dir, up]: mDirUpPairs) {
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + dir, up));
	}

	mDepthShader->activate();
	for (uint32_t i = 0; i < 6; ++i) {
		mDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	}

	mDepthShader->setFloat("omniFarPlane", ctx.shadow.omnidirectional.farPlane);
	mDepthShader->setVec3("lightPos", position);
	mDepthShader->setInt("cubeIndex", layer);

	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->shadowingObjects) {
		RenderCommon::setupTransform(*entity, *mDepthShader);
		RenderCommon::drawMesh(mesh->vao(), mesh->vertices().size(), mesh->indices().size());
	}
}
