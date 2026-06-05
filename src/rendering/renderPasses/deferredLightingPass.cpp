#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../mesh/mesh.h"
#include "../renderCommand.h"
#include "../renderContext/renderContext.hpp"

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mShader = ctx.resourceManager->get<Shader>("deferredLighting");

	const std::vector<TextureBinding> textureBindings = {
		{"gPosition", ctx.gBuffer.position.textureSlot},
		{"gNormal", ctx.gBuffer.normal.textureSlot},
		{"gAlbedo", ctx.gBuffer.albedo.textureSlot},
		{"gORM", ctx.gBuffer.orm.textureSlot},
		{"ssao", ctx.ssao.textureSlot},
		{"irradianceMap", ctx.PBR.irradianceMap.textureSlot},
		{"prefilterMap", ctx.PBR.prefilterMap.textureSlot},
		{"brdfLUT", ctx.PBR.brdfLUT.textureSlot}
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
	RenderCommand::bindShadowMaps(ctx);

	ctx.gBuffer.buffer->bindTexture(ctx.gBuffer.position.textureSlot, ctx.gBuffer.position.textureIdx);
	ctx.gBuffer.buffer->bindTexture(ctx.gBuffer.normal.textureSlot, ctx.gBuffer.normal.textureIdx);
	ctx.gBuffer.buffer->bindTexture(ctx.gBuffer.albedo.textureSlot, ctx.gBuffer.albedo.textureIdx);
	ctx.gBuffer.buffer->bindTexture(ctx.gBuffer.orm.textureSlot, ctx.gBuffer.orm.textureIdx);
	ctx.ssao.buffer->bindTexture(ctx.ssao.textureSlot);
	ctx.PBR.irradianceMap.buffer->bindTexture(ctx.PBR.irradianceMap.textureSlot);
	ctx.PBR.prefilterMap.buffer->bindTexture(ctx.PBR.prefilterMap.textureSlot);
	ctx.PBR.brdfLUT.buffer->bindTexture(ctx.PBR.brdfLUT.textureSlot);
}

void DeferredLightingPass::execute(const RenderContext& ctx) {
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	ctx.gBuffer.buffer->bindForRead();
	ctx.sceneBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, ctx.gBuffer.buffer->width(), ctx.gBuffer.buffer->height(),
	                  0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
	                  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ctx.sceneBuffer->bind();
	mShader->activate();

	RenderCommand::drawQuad(mQuad->vao());
}
