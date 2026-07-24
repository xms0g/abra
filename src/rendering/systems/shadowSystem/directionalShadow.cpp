#include "directionalShadow.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "../../mesh/mesh.h"
#include "../../shader.h"
#include "../../renderGraph.h"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderGroup.hpp"
#include "../../renderContext/renderData.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommand.h"
#include "../../renderGraph.h"
#include "../../../config/configManager.h"

DirectionalShadow::DirectionalShadow(const RenderContext& ctx) {
	mDepthShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("depth");
	mObjects = &ctx.queueRegistry->get<RenderGroup>("shadow");

	mHeight = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.height");
	mRight = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.right");
	mLeft = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.left");
	mTop = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.top");
	mBottom = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.bottom");
	mNear = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.nearPlane");
	mFar = CONFIG_MANAGER_INSTANCE.get<float>("shadow.directional.farPlane");
}

DirectionalShadow::~DirectionalShadow() = default;

glm::mat4 DirectionalShadow::lightSpaceMatrix() const {
	return mLightSpaceMatrix;
}

void DirectionalShadow::render(const RenderContext& ctx, const RenderGraph& graph, const glm::vec3& direction) {
	const glm::vec3 lightPos = -direction * mHeight;
	const glm::mat4 lightProjection = glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
	const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	mLightSpaceMatrix = lightProjection * lightView;

	mDepthShader->bind();
	mDepthShader->setMat4("lightSpaceMatrix", mLightSpaceMatrix);

	// render scene from light's point of view
	graph.getResource("directional").bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	for (const auto& [entityID, matBatch]: *mObjects) {
		RenderCommand::setupTransform(entityID, ctx, *mDepthShader);

		for (const auto& meshIdx: matBatch.meshIndices) {
			const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderData->mesh.indexCounts[meshIdx];

			RenderCommand::drawMesh(vao, vertexCount, indexCount);
		}
	}
	graph.getResource("directional").unbind();
}
