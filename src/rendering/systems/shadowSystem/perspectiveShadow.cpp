#include "perspectiveShadow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../context/renderGroup.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderQueue.hpp"
#include "../../renderCommand.h"
#include "../../../config/configManager.h"

PerspectiveShadow::PerspectiveShadow(const RenderContext& ctx) {
	mWidth = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width");
	mHeight = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height");
	mAspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
	mDepthShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("depth");
	mObjects = std::span(
		ctx.queueRegistry->get<RenderGroup>("shadow").data(),
		ctx.queueRegistry->get<RenderGroup>("shadow").size());
	mNear = CONFIG_MANAGER_INSTANCE.get<float>("shadow.perspective.nearPlane");
	mFar = CONFIG_MANAGER_INSTANCE.get<float>("shadow.perspective.farPlane");
}

PerspectiveShadow::~PerspectiveShadow() = default;

glm::mat4 PerspectiveShadow::lightSpaceMatrix(const int layer) const {
	return mLightSpaceMatrix[layer];
}

void PerspectiveShadow::render(
	const RenderContext& ctx,
	const glm::vec3& direction,
	const glm::vec3& position,
	const float fovy,
	const uint32_t texture,
	const int32_t layer) {
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0, layer);

	glClear(GL_DEPTH_BUFFER_BIT);

	const glm::mat4 lightProjection = glm::perspective(fovy, mAspect, mNear, mFar);
	const glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
	mLightSpaceMatrix[layer] = lightProjection * lightView;

	mDepthShader->bind();
	mDepthShader->setMat4("lightSpaceMatrix", mLightSpaceMatrix[layer]);

	for (const auto& [entityID, matBatch]: mObjects) {
		RenderCommand::setupTransform(entityID, ctx, *mDepthShader);

		for (const auto& meshIdx: matBatch.meshIndices) {
			const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderData->mesh.indexCounts[meshIdx];

			RenderCommand::drawMesh(vao, vertexCount, indexCount);
		}
	}
}
