#include "deferredGeometryPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../ECS/components/bv.hpp"
#include "../../ECS/entity.hpp"

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::configure(const RenderContext& ctx) {
	mGBuffer = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mGBuffer->withTextureFP(true, 16)
			.withTextureFP(true, 16)
#ifdef HDR
			.withTextureFP(true, 16)
#else
			.withTexture()
#endif
			.configureAttachments()
			.withRenderBufferDepth(24)
			.checkStatus();

	mShader = std::make_unique<Shader>("deferred/gbuffer.vert", "deferred/gbuffer.frag");
	ctx.camera.ubo->configure(mShader->ID(), 0, "CameraBlock");
}

void DeferredGeometryPass::execute(const RenderContext& ctx) {
	mGBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mShader->activate();
	for (const auto& [entity, matBatches]: ctx.renderQueue->deferredGroups) {
		if (!entity->getComponent<BoundingVolumeComponent>().isVisible)
			continue;

		for (const auto& [material, shader, meshes]: matBatches) {
			RenderCommon::setupTransform(*entity, *mShader);
			RenderCommon::setupMaterial(*entity, *mShader);

			RenderCommon::bindTextures(material->textures, *mShader);
			RenderCommon::drawMeshes(*meshes);
			RenderCommon::unbindTextures(material->textures);
		}
	}
	mGBuffer->unbind();
}
