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
#include "../../../rendering/graphicsEncoder.h"
#include "../../../rendering/graphicsPipeline.h"

PerspectiveShadow::PerspectiveShadow(const RenderContext& ctx) {
	mWidth = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_width");
	mHeight = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.map_height");
	mAspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
	mNear = CONFIG_MANAGER_INSTANCE.get<float>("shadow.perspective.nearPlane");
	mFar = CONFIG_MANAGER_INSTANCE.get<float>("shadow.perspective.farPlane");

	mObjects = std::span(
		ctx.queueRegistry->get<RenderGroup>("shadow").data(),
		ctx.queueRegistry->get<RenderGroup>("shadow").size());
}

PerspectiveShadow::~PerspectiveShadow() = default;

glm::mat4 PerspectiveShadow::lightSpaceMatrix(const int layer) const {
	return mLightSpaceMatrix[layer];
}

void PerspectiveShadow::render(
	const RenderContext& ctx,
	GraphicsEncoder& encoder,
	GraphicsPipeline& pipeline,
	const FrameBuffer& frameBuffer,
	const glm::vec3& direction,
	const glm::vec3& position,
	const float fovy,
	const int32_t layer) {
	encoder.attachFramebufferTextureLayer(frameBuffer, layer);
	encoder.clearFrameBuffer(ClearMask::Depth);

	const glm::mat4 lightProjection = glm::perspective(fovy, mAspect, mNear, mFar);
	const glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
	mLightSpaceMatrix[layer] = lightProjection * lightView;

	encoder.bindPipeline(pipeline);
	encoder.setUniform("lightSpaceMatrix", mLightSpaceMatrix[layer]);

	for (const auto& [entityID, matBatch]: mObjects) {
		encoder.bindTransform({
			.model = ctx.renderData->entity.models[entityID],
			.normal = ctx.renderData->entity.normals[entityID],
		});

		for (const auto& meshIdx: matBatch.meshIndices) {
			encoder.drawIndexed({
				.vao = ctx.renderData->mesh.vaos[meshIdx],
				.vertexCount = 0,
				.indexCount = ctx.renderData->mesh.indexCounts[meshIdx]
			});
		}
	}
}
