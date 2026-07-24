#include "omnidirectionalShadow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../context/renderGroup.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommand.h"
#include "../../../config/configManager.h"

OmnidirectionalShadow::OmnidirectionalShadow(const RenderContext& ctx) {
	mWidth = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width");
	mHeight = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height");
	mAspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
	mDepthShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("depthCubemap");
	mObjects = &ctx.queueRegistry->get<RenderGroup>("shadow");

	mNear = CONFIG_MANAGER_INSTANCE.get<float>("shadow.omnidirectional.nearPlane");
	mFar = CONFIG_MANAGER_INSTANCE.get<float>("shadow.omnidirectional.farPlane");
	mFovy = glm::radians(CONFIG_MANAGER_INSTANCE.get<float>("shadow.omnidirectional.fovy"));
	mShadowProj = glm::perspective(mFovy, mAspect, mNear, mFar);

	mShadowTransforms.resize(faces);
}

OmnidirectionalShadow::~OmnidirectionalShadow() = default;

void OmnidirectionalShadow::render(
	const RenderContext& ctx,
	const glm::vec3& position,
	const int32_t layer) {
	mShadowTransforms.clear();

	for (const auto& [dir, up]: mDirUps) {
		mShadowTransforms.push_back(mShadowProj * glm::lookAt(position, position + dir, up));
	}

	mDepthShader->bind();
	mDepthShader->setMat4Array("shadowMatrices", mShadowTransforms.data(), mShadowTransforms.size());
	mDepthShader->setFloat("omniFarPlane", mFar);
	mDepthShader->setVec3("lightPos", position);
	mDepthShader->setInt("cubeIndex", layer);

	for (const auto& [entityID, matBatch]: *mObjects) {
		RenderCommand::setupTransform(entityID, ctx, *mDepthShader);

		for (const auto& meshIdx: matBatch.meshIndices) {
			const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderData->mesh.indexCounts[meshIdx];

			RenderCommand::drawMesh(vao, vertexCount, indexCount);
		}
	}
}
