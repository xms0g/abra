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
		{"gPosition", ConfigManager::instance().get<int32_t>("gBuffer.position.textureSlot")},
		{"gNormal", ConfigManager::instance().get<int32_t>("gBuffer.normal.textureSlot")},
		{"gAlbedo", ConfigManager::instance().get<int32_t>("gBuffer.albedo.textureSlot")},
		{"gORM", ConfigManager::instance().get<int32_t>("gBuffer.orm.textureSlot")},
		{"ssao", ConfigManager::instance().get<int32_t>("ssao.textureSlot")},
		{"irradianceMap", ConfigManager::instance().get<int32_t>("PBR.irradianceMap.textureSlot")},
		{"prefilterMap", ConfigManager::instance().get<int32_t>("PBR.prefilterMap.textureSlot")},
		{"brdfLUT", ConfigManager::instance().get<int32_t>("PBR.brdfLUT.textureSlot")},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
	RenderCommand::bindShadowMaps(ctx);

	ctx.gBuffer->bindTexture(ConfigManager::instance().get<int32_t>("gBuffer.position.textureSlot"), ConfigManager::instance().get<int32_t>("gBuffer.position.textureIdx"));
	ctx.gBuffer->bindTexture(ConfigManager::instance().get<int32_t>("gBuffer.normal.textureSlot"), ConfigManager::instance().get<int32_t>("gBuffer.normal.textureIdx"));
	ctx.gBuffer->bindTexture(ConfigManager::instance().get<int32_t>("gBuffer.albedo.textureSlot"), ConfigManager::instance().get<int32_t>("gBuffer.albedo.textureIdx"));
	ctx.gBuffer->bindTexture(ConfigManager::instance().get<int32_t>("gBuffer.orm.textureSlot"), ConfigManager::instance().get<int32_t>("gBuffer.orm.textureIdx"));
	ctx.ssao.buffer->bindTexture(ConfigManager::instance().get<int32_t>("ssao.textureSlot"));
	ctx.PBR.irradianceMap->bindTexture(ConfigManager::instance().get<int32_t>("PBR.irradianceMap.textureSlot"));
	ctx.PBR.prefilterMap->bindTexture(ConfigManager::instance().get<int32_t>("PBR.prefilterMap.textureSlot"));
	ctx.PBR.brdfLUT->bindTexture(ConfigManager::instance().get<int32_t>("PBR.brdfLUT.textureSlot"));
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
