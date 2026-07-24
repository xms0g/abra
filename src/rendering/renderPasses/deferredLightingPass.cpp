#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../renderGraph.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../../config/configManager.h"

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) {
	mQuad = std::make_unique<Model::SingleQuad>();
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("deferredLighting");

	const TextureBinding textureBindings[] = {
		{.name = "gPosition", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureSlot")},
		{.name = "gNormal", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot")},
		{.name = "gAlbedo", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureSlot")},
		{.name = "gORM", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureSlot")},
		{.name = "ssao", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.textureSlot")},
		{.name = "irradianceMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.textureSlot")},
		{.name = "prefilterMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.textureSlot")},
		{.name = "brdfLUT", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.textureSlot")},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);

	const auto& gBuffer = graph.getResource("gBuffer");
	const auto& ssaoBlur = graph.getResource("ssaoBlur");

	gBuffer.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureIdx"));
	gBuffer.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureIdx"));
	gBuffer.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureIdx"));
	gBuffer.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureIdx"));
	ssaoBlur.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.textureSlot"));
	RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>("irradianceMap")->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.textureSlot"));
	RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>("prefilterMap")->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.textureSlot"));
	RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>("brdfLUT")->bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.textureSlot"));

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	graph.getResource("directional").bindTexture(slot);
	graph.getResource("point").bindTexture(slot + 1);
	graph.getResource("spot").bindTexture(slot + 2);
}

void DeferredLightingPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	const auto& sceneBuffer = graph.getResource("sceneBuffer");
	const auto& gBuffer = graph.getResource("gBuffer");
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	gBuffer.bindForRead();
	sceneBuffer.bindForDraw();

	glBlitFramebuffer(0, 0, gBuffer.width(), gBuffer.height(),
	                  0, 0, sceneBuffer.width(), sceneBuffer.height(),
	                  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	sceneBuffer.bind();
	mShader->activate();

	RenderCommand::drawQuad(mQuad->vao());
}
