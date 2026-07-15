#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers//vertexBuffer.h"
#include "../mesh/mesh.h"
#include "../mesh/vertexArray.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../../config/configManager.h"

DeferredLightingPass::DeferredLightingPass()= default;

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Model::SingleQuad>();
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("deferredLighting");

	const TextureBinding textureBindings[] = {
		{"gPosition", CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureSlot")},
		{"gNormal", CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot")},
		{"gAlbedo", CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureSlot")},
		{"gORM", CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureSlot")},
		{"ssao", CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.textureSlot")},
		{"irradianceMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.textureSlot")},
		{"prefilterMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.textureSlot")},
		{"brdfLUT", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.textureSlot")},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);

	ctx.gBuffer->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureIdx"));
	ctx.gBuffer->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureIdx"));
	ctx.gBuffer->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureIdx"));
	ctx.gBuffer->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureIdx"));
	ctx.ssao.buffer->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.textureSlot"));
	RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>("irradianceMap")->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.textureSlot"));
	RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>("prefilterMap")->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.textureSlot"));
	RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>("brdfLUT")->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.textureSlot"));

	RenderCommand::bindShadowMaps(ctx);
}

void DeferredLightingPass::execute(const RenderContext& ctx) {
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	ctx.gBuffer->bindForRead();
	ctx.sceneBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, ctx.gBuffer->width(), ctx.gBuffer->height(),
	                  0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
	                  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ctx.sceneBuffer->bind();
	mShader->activate();

	RenderCommand::drawQuad(mQuad->vao());
}
