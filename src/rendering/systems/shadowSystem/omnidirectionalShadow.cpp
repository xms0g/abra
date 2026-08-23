#include "omnidirectionalShadow.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../context/renderGroup.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderQueue.hpp"
#include "../../../config/configManager.hpp"
#include "../../../rendering/graphicsEncoder.hpp"

OmnidirectionalShadow::OmnidirectionalShadow(const RenderContext& ctx) {
	mWidth = CONFIG_MANAGER.get<int32_t>("shadow.map.width");
	mHeight = CONFIG_MANAGER.get<int32_t>("shadow.map.height");
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

void OmnidirectionalShadow::render(const RenderContext& ctx,
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
		encoder.setUniform("model", ctx.renderData->entity.models[entityID]);
		encoder.setUniform("normalMatrix", ctx.renderData->entity.normals[entityID]);

		for (const auto& meshIdx: matBatch.meshIndices) {
			encoder.bindVertexArray(ctx.renderData->mesh.vaos[meshIdx]);
			encoder.drawIndexed(ctx.renderData->mesh.indexCounts[meshIdx]);
		}
	}
}
