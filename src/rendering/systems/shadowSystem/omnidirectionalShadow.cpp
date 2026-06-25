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
	mAspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
	mDepthMap = std::make_unique<FrameBuffer>(mWidth, mHeight);
	mDepthMap->withTextureCubemapDepthArray(cfg.get<int32_t>("light.max_point"), GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = rm.get<Shader>("depthCubemap");
	mNear = cfg.get<float>("shadow.omnidirectional.nearPlane");
	mFar = cfg.get<float>("shadow.omnidirectional.farPlane");
	mFovy = glm::radians(cfg.get<float>("shadow.omnidirectional.fovy"));
	mShadowProj = glm::perspective(mFovy, mAspect, mNear, mFar);

	mShadowTransforms.resize(faces);
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
	const int32_t layer) {
	mShadowTransforms.clear();

	for (const auto& [dir, up]: mDirUps) {
		mShadowTransforms.push_back(mShadowProj * glm::lookAt(position, position + dir, up));
	}

	mDepthShader->activate();
	mDepthShader->setMat4Array("shadowMatrices", mShadowTransforms.data(), mShadowTransforms.size());
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
