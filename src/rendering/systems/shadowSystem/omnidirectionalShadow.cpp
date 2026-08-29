#include "omnidirectionalShadow.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../pushConstants/transformPushConstants.hpp"
#include "../../buffers/uniformBuffer.hpp"
#include "../../context/renderGroup.hpp"
#include "../../context/renderData.hpp"
#include "../../context/renderContext.hpp"
#include "../../context/renderQueue.hpp"
#include "../../../config/configManager.hpp"
#include "../../../rendering/graphicsEncoder.hpp"

OmnidirectionalShadow::OmnidirectionalShadow(const RenderContext& ctx) {
	const int32_t width = CONFIG_MANAGER.get<int32_t>("shadow.map.width");
	const int32_t height = CONFIG_MANAGER.get<int32_t>("shadow.map.height");
	const float aspect = static_cast<float>(width) / static_cast<float>(height);
	const float near = CONFIG_MANAGER.get<float>("shadow.omnidirectional.nearPlane");
	const float far = CONFIG_MANAGER.get<float>("shadow.omnidirectional.farPlane");
	const float fovy = glm::radians(CONFIG_MANAGER.get<float>("shadow.omnidirectional.fovy"));

	mShadowProj = glm::perspective(fovy, aspect, near, far);

	mObjects = std::span(
		ctx.queueRegistry->get<RenderGroup>("shadow").data(),
		ctx.queueRegistry->get<RenderGroup>("shadow").size());

	mData.posFarPlane.w = far;
}

OmnidirectionalShadow::~OmnidirectionalShadow() = default;

void OmnidirectionalShadow::render(const RenderContext& ctx,
                                   GraphicsEncoder& encoder,
                                   GraphicsPipeline& pipeline,
                                   const UniformBuffer& ubo,
                                   const glm::vec3& position,
                                   const int32_t layer) {
	for (int32_t i = 0; i < faces; ++i) {
		const auto& [dir, up] = mDirUps[i];
		mData.shadowMatrices[i] = mShadowProj * glm::lookAt(position, position + dir, up);
	}

	mData.posFarPlane.x = position.x;
	mData.posFarPlane.y = position.y;
	mData.posFarPlane.z = position.z;

	ubo.copyToMemory(&mData, offsetof(ShadowData, omniShadowData), sizeof(OmnidirectionalShadowData));

	encoder.bindPipeline(pipeline);
	encoder.setUniform("cubeIndex", layer);

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
