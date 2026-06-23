#include "omnidirectionalShadow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderGroup.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../renderContext/renderContext.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommand.h"
#include "../../../config/configManager.h"

OmnidirectionalShadow::OmnidirectionalShadow(const RenderContext& ctx) {
	mWidth = cfg.get<int32_t>("shadow.map_width");
	mHeight = cfg.get<int32_t>("shadow.map_height");
	mDepthMap = std::make_unique<FrameBuffer>(mWidth, mHeight);
	mDepthMap->withTextureCubemapDepthArray(cfg.get<int32_t>("light.max_point"), GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = rm.get<Shader>("depthCubemap");
	mNear = cfg.get<float>("shadow.omnidirectional.nearPlane");
	mFar = cfg.get<float>("shadow.omnidirectional.farPlane");
	mFovy = glm::radians(cfg.get<float>("shadow.omnidirectional.fovy"));
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
		glm::radians(mFovy),
		static_cast<float>(mWidth) / static_cast<float>(mHeight),
		mNear,
		mFar);

	std::vector<glm::mat4> shadowTransforms;
	for (const auto& [dir, up]: mDirUpPairs) {
		shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + dir, up));
	}

	mDepthShader->activate();
	for (uint32_t i = 0; i < faces; ++i) {
		mDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	}

	mDepthShader->setFloat("omniFarPlane", mFar);
	mDepthShader->setVec3("lightPos", position);
	mDepthShader->setInt("cubeIndex", layer);

	for (const auto& [entityID, matBatch]: ctx.renderQueue->shadowGroups) {
		RenderCommand::setupTransform(entityID, ctx, *mDepthShader);

		for (const auto& meshIdx: matBatch.meshIndices) {
			const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

			RenderCommand::drawMesh(vao, vertexCount, indexCount);
		}
	}
}
