#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../mesh/mesh.h"
#include "../renderCommand.h"
#include "../renderContext/renderContext.hpp"
#include "../../config/configManager.h"

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Model::SingleQuad>();
	mShader = ResourceManager::instance().get<Shader>("deferredLighting");

	const std::vector<TextureBinding> textureBindings = {
		{"gPosition", ConfigManager::instance().gBuffer.position.textureSlot},
		{"gNormal", ConfigManager::instance().gBuffer.normal.textureSlot},
		{"gAlbedo", ConfigManager::instance().gBuffer.albedo.textureSlot},
		{"gORM", ConfigManager::instance().gBuffer.orm.textureSlot},
		{"ssao", ConfigManager::instance().ssao.textureSlot},
		{"irradianceMap", ConfigManager::instance().PBR.irradianceMap.textureSlot},
		{"prefilterMap", ConfigManager::instance().PBR.prefilterMap.textureSlot},
		{"brdfLUT", ConfigManager::instance().PBR.brdfLUT.textureSlot}
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
	RenderCommand::bindShadowMaps(ctx);

	ctx.gBuffer->bindTexture(ConfigManager::instance().gBuffer.position.textureSlot, ConfigManager::instance().gBuffer.position.textureIdx);
	ctx.gBuffer->bindTexture(ConfigManager::instance().gBuffer.normal.textureSlot, ConfigManager::instance().gBuffer.normal.textureIdx);
	ctx.gBuffer->bindTexture(ConfigManager::instance().gBuffer.albedo.textureSlot, ConfigManager::instance().gBuffer.albedo.textureIdx);
	ctx.gBuffer->bindTexture(ConfigManager::instance().gBuffer.orm.textureSlot, ConfigManager::instance().gBuffer.orm.textureIdx);
	ctx.ssao.buffer->bindTexture(ConfigManager::instance().ssao.textureSlot);
	ctx.PBR.irradianceMap->bindTexture(ConfigManager::instance().PBR.irradianceMap.textureSlot);
	ctx.PBR.prefilterMap->bindTexture(ConfigManager::instance().PBR.prefilterMap.textureSlot);
	ctx.PBR.brdfLUT->bindTexture(ConfigManager::instance().PBR.brdfLUT.textureSlot);
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
