#include "omnidirectionalShadow.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../context/renderGroup.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderQueue.hpp"
#include "../../../config/configManager.h"
#include "../../../rendering/graphicsEncoder.h"
#include "../../../rendering/graphicsPipeline.h"

OmnidirectionalShadow::OmnidirectionalShadow(const RenderContext& ctx) {
	mWidth = CONFIG_MANAGER.get<int32_t>("shadow.map_width");
	mHeight = CONFIG_MANAGER.get<int32_t>("shadow.map_height");
	mAspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
	mNear = CONFIG_MANAGER.get<float>("shadow.omnidirectional.nearPlane");
	mFar = CONFIG_MANAGER.get<float>("shadow.omnidirectional.farPlane");
	mFovy = glm::radians(CONFIG_MANAGER.get<float>("shadow.omnidirectional.fovy"));
	mShadowProj = glm::perspective(mFovy, mAspect, mNear, mFar);
	mShadowTransforms.resize(faces);
	mObjects = std::span(
		ctx.queueRegistry->get<RenderGroup>("shadow").data(),
		ctx.queueRegistry->get<RenderGroup>("shadow").size());
}

OmnidirectionalShadow::~OmnidirectionalShadow() = default;

void OmnidirectionalShadow::render(
	const RenderContext& ctx,
	GraphicsEncoder& encoder,
	GraphicsPipeline& pipeline,
	const glm::vec3& position,
	const int32_t layer) {
	mShadowTransforms.clear();

	for (const auto& [dir, up]: mDirUps) {
		mShadowTransforms.push_back(mShadowProj * glm::lookAt(position, position + dir, up));
	}

	encoder.bindPipeline(pipeline);
	encoder.setUniform("shadowMatrices", mShadowTransforms.data(), mShadowTransforms.size());
	encoder.setUniform("omniFarPlane", mFar);
	encoder.setUniform("lightPos", position);
	encoder.setUniform("cubeIndex", layer);

	for (const auto& [entityID, matBatch]: mObjects) {
		encoder.bindTransform({
			.model = ctx.renderData->entity.models[entityID],
			.normal = ctx.renderData->entity.normals[entityID],
		});

		for (const auto& meshIdx: matBatch.meshIndices) {
			encoder.bindVertexArray(ctx.renderData->mesh.vaos[meshIdx]);
			encoder.drawIndexed(ctx.renderData->mesh.indexCounts[meshIdx]);
		}
	}
}
