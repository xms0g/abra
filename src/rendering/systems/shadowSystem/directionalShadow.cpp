#include "directionalShadow.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../mesh/mesh.hpp"
#include "../../buffers/uniformBuffer.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderGroup.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderQueue.hpp"
#include "../../../config/configManager.hpp"
#include "../../../rendering/graphicsEncoder.hpp"
#include "../../pushConstants/transformPushConstants.hpp"

DirectionalShadow::DirectionalShadow(const RenderContext& ctx) {
	mHeight = CONFIG_MANAGER.get<float>("shadow.directional.height");
	const float right = CONFIG_MANAGER.get<float>("shadow.directional.right");
	const float left = CONFIG_MANAGER.get<float>("shadow.directional.left");
	const float top = CONFIG_MANAGER.get<float>("shadow.directional.top");
	const float bottom = CONFIG_MANAGER.get<float>("shadow.directional.bottom");
	const float near = CONFIG_MANAGER.get<float>("shadow.directional.nearPlane");
	const float far = CONFIG_MANAGER.get<float>("shadow.directional.farPlane");

	mLightProjection = glm::ortho(left, right, bottom, top, near, far);

	mObjects = std::span(
		ctx.queueRegistry->get<RenderGroup>("shadow").data(),
		ctx.queueRegistry->get<RenderGroup>("shadow").size());
}

DirectionalShadow::~DirectionalShadow() = default;

void DirectionalShadow::render(const RenderContext& ctx,
                               GraphicsEncoder& encoder,
                               GraphicsPipeline& pipeline,
                               const UniformBuffer& ubo,
                               const glm::vec3& direction) {
	const glm::vec3 lightPos = -direction * mHeight;
	const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	mData.lightSpaceMatrix = mLightProjection * lightView;
	ubo.copyToMemory(&mData, offsetof(ShadowData, dirShadowData), sizeof(DirectionalShadowData));

	encoder.bindPipeline(pipeline);

	for (const auto& [entityID, matBatch]: mObjects) {
		{
			TransformPushConstants transformPushConstants = {
				.model = ctx.renderData->entity.models[entityID],
				.normal = ctx.renderData->entity.normals[entityID]
			};

			encoder.pushConstants(&transformPushConstants);
		}

		for (const auto& meshIdx: matBatch.meshIndices) {
			encoder.bindVertexArray(ctx.renderData->mesh.vaos[meshIdx]);
			encoder.drawIndexed(ctx.renderData->mesh.indexCounts[meshIdx]);
		}
	}
}
