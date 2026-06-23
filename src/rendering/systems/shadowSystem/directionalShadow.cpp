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
#include "../../../config/configManager.h"

DirectionalShadow::DirectionalShadow(const RenderContext& ctx) {
	mDepthMap = std::make_unique<FrameBuffer>(
		cfg.get<int32_t>("shadow.map_width"),
		cfg.get<int32_t>("shadow.map_height"));
	mDepthMap->withTextureDepth(GL_DEPTH_COMPONENT24, true)
			.checkStatus();
	mDepthMap->unbind();

	mDepthShader = rm.get<Shader>("depth");

	mHeight = cfg.get<float>("shadow.directional.height");
	mRight = cfg.get<float>("shadow.directional.right");
	mLeft = cfg.get<float>("shadow.directional.left");
	mTop = cfg.get<float>("shadow.directional.top");
	mBottom = cfg.get<float>("shadow.directional.bottom");
	mNear = cfg.get<float>("shadow.directional.nearPlane");
	mFar = cfg.get<float>("shadow.directional.farPlane");
}

DirectionalShadow::~DirectionalShadow() = default;

uint32_t DirectionalShadow::depthTexture() const {
	return mDepthMap->texture();
}

glm::mat4 DirectionalShadow::lightSpaceMatrix() const {
	return mLightSpaceMatrix;
}

void DirectionalShadow::render(const RenderContext& ctx, const glm::vec3& direction) {
	const glm::vec3 lightPos = -direction * mHeight;
	const glm::mat4 lightProjection = glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
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
