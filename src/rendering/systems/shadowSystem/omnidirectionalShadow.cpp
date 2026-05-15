#include "omnidirectionalShadow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderGroup.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../renderContext/renderContext.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"

OmnidirectionalShadow::OmnidirectionalShadow(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureCubemapDepthArray(ctx.shadow.omnidirectional.maxLights, GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>(
		"depth/depthCubemap.vert",
		"depth/depthCubemap.frag",
		"depth/depthCubemap.geom");
}

OmnidirectionalShadow::~OmnidirectionalShadow() = default;

uint32_t OmnidirectionalShadow::depthTexture() const {
	return mDepthMap->texture();
}

FrameBuffer& OmnidirectionalShadow::depthMap() const {
	return *mDepthMap;
}

void OmnidirectionalShadow::render(
	const RenderContext& ctx,
	const glm::vec3& position,
	const int32_t layer) const {
	constexpr uint32_t faces = 6;

	const glm::mat4 shadowProj = glm::perspective(
		glm::radians(ctx.shadow.omnidirectional.fovy),
		static_cast<float>(ctx.shadow.width) / static_cast<float>(ctx.shadow.height),
		ctx.shadow.omnidirectional.nearPlane,
		ctx.shadow.omnidirectional.farPlane);

	std::vector<glm::mat4> shadowTransforms;
	for (const auto& [dir, up]: mDirUpPairs) {
		shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + dir, up));
	}

	mDepthShader->activate();
	for (uint32_t i = 0; i < faces; ++i) {
		mDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	}

	mDepthShader->setFloat("omniFarPlane", ctx.shadow.omnidirectional.farPlane);
	mDepthShader->setVec3("lightPos", position);
	mDepthShader->setInt("cubeIndex", layer);

	for (const auto& [entityID, matBatch]: ctx.renderQueue->shadowGroups) {
		RenderCommon::setupTransform(entityID, ctx, *mDepthShader);

		const auto& [matIdx, shader, meshes] = matBatch;

		for (const auto& meshIdx: meshes) {
			const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

			RenderCommon::drawMesh(vao, vertexCount, indexCount);
		}
	}
}
