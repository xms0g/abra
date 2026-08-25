#include "directionalShadow.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../mesh/mesh.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderGroup.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderQueue.hpp"
#include "../../../config/configManager.hpp"
#include "../../../rendering/graphicsEncoder.hpp"
#include "../../pushConstants/transformPushConstants.hpp"

DirectionalShadow::DirectionalShadow(const RenderContext& ctx) {
	mHeight = CONFIG_MANAGER.get<float>("shadow.directional.height");
	mRight = CONFIG_MANAGER.get<float>("shadow.directional.right");
	mLeft = CONFIG_MANAGER.get<float>("shadow.directional.left");
	mTop = CONFIG_MANAGER.get<float>("shadow.directional.top");
	mBottom = CONFIG_MANAGER.get<float>("shadow.directional.bottom");
	mNear = CONFIG_MANAGER.get<float>("shadow.directional.nearPlane");
	mFar = CONFIG_MANAGER.get<float>("shadow.directional.farPlane");

	mObjects = std::span(
		ctx.queueRegistry->get<RenderGroup>("shadow").data(),
		ctx.queueRegistry->get<RenderGroup>("shadow").size());
}

DirectionalShadow::~DirectionalShadow() = default;

glm::mat4 DirectionalShadow::lightSpaceMatrix() const {
	return mLightSpaceMatrix;
}

void DirectionalShadow::render(const RenderContext& ctx,
                               GraphicsEncoder& encoder,
                               GraphicsPipeline& pipeline,
                               const glm::vec3& direction) {
	const glm::vec3 lightPos = -direction * mHeight;
	const glm::mat4 lightProjection = glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
	const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	mLightSpaceMatrix = lightProjection * lightView;

	encoder.bindPipeline(pipeline);
	encoder.setUniform("lightSpaceMatrix", mLightSpaceMatrix);

	for (const auto& [entityID, matBatch]: mObjects) {
		TransformPushConstants transformPushConstants = {
			.model = ctx.renderData->entity.models[entityID],
			.normal = ctx.renderData->entity.normals[entityID]
		};

		encoder.pushConstants(&transformPushConstants);

		for (const auto& meshIdx: matBatch.meshIndices) {
			encoder.bindVertexArray(ctx.renderData->mesh.vaos[meshIdx]);
			encoder.drawIndexed(ctx.renderData->mesh.indexCounts[meshIdx]);
		}
	}
	encoder.unbindFrameBuffer();
}
