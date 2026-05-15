#include "perspectiveShadow.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../shader.h"
#include "../../renderContext/renderGroup.hpp"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/frameBuffer.h"
#include "../../renderCommon.h"

PerspectiveShadow::PerspectiveShadow(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(ctx.shadow.width, ctx.shadow.height);
	mDepthMap->withTextureDepthArray(ctx.shadow.perspective.maxLights, GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = std::make_unique<Shader>("depth/depth.vert", "depth/depth.frag");
}

PerspectiveShadow::~PerspectiveShadow() = default;

uint32_t PerspectiveShadow::depthTexture() const {
	return mDepthMap->texture();
}

FrameBuffer& PerspectiveShadow::depthMap() const {
	return *mDepthMap;
}

glm::mat4 PerspectiveShadow::lightSpaceMatrix(const int layer) const {
	return mLightSpaceMatrix[layer];
}

void PerspectiveShadow::render(
	const RenderContext& ctx,
	const glm::vec3& direction,
	const glm::vec3& position,
	const float fovy,
	const int32_t layer) {
	glFramebufferTextureLayer(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		mDepthMap->texture(),
		0,
		layer);

	glClear(GL_DEPTH_BUFFER_BIT);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthMap->texture(), layer);

	const glm::mat4 lightProjection = glm::perspective(
		fovy,
		static_cast<float>(ctx.shadow.width) / static_cast<float>(ctx.shadow.height),
		ctx.shadow.perspective.nearPlane,
		ctx.shadow.perspective.farPlane);

	const glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
	mLightSpaceMatrix[layer] = lightProjection * lightView;

	mDepthShader->activate();
	mDepthShader->setMat4("lightSpaceMatrix", mLightSpaceMatrix[layer]);

	for (const auto& [entityID, matBatch]: ctx.renderQueue->shadowGroups) {
		RenderCommon::setupTransform(entityID, ctx, *mDepthShader);

		const auto& [matIdx, shader, meshes] = matBatch;

		for (const auto& meshIdx: meshes) {
			const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
			const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
			const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

			RenderCommon::drawMesh(vao, vertexCount, indexCount);
		}
	}
}
