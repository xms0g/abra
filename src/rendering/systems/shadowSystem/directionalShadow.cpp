#include "directionalShadow.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "../../mesh/mesh.h"
#include "../../shader.h"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderGroup.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommand.h"

DirectionalShadow::DirectionalShadow(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureDepth(GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = ctx.resourceManager->get<Shader>("depth");
}

DirectionalShadow::~DirectionalShadow() = default;

uint32_t DirectionalShadow::depthTexture() const {
	return mDepthMap->texture();
}

glm::mat4 DirectionalShadow::lightSpaceMatrix() const {
	return mLightSpaceMatrix;
}

void DirectionalShadow::render(const RenderContext& ctx, const glm::vec3& direction) {
	const glm::vec3 lightPos = -direction * ctx.shadow.directional.height;
	const glm::mat4 lightProjection = glm::ortho(
		ctx.shadow.directional.left,
		ctx.shadow.directional.right,
		ctx.shadow.directional.bottom,
		ctx.shadow.directional.top,
		ctx.shadow.directional.nearPlane,
		ctx.shadow.directional.farPlane);

	const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	mLightSpaceMatrix = lightProjection * lightView;

	mDepthShader->activate();
	mDepthShader->setMat4("lightSpaceMatrix", mLightSpaceMatrix);

	// render scene from light's point of view
	mDepthMap->bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	for (const auto& [entityID, matBatch]: ctx.renderQueue->shadowGroups) {
		RenderCommand::setupTransform(entityID, ctx, *mDepthShader);

		for (const auto& meshIdx: matBatch.meshIndices) {
			const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

			RenderCommand::drawMesh(vao, vertexCount, indexCount);
		}
	}
	mDepthMap->unbind();
}
