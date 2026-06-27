#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../texture/texture.h"

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(RenderContext& ctx, EventBus& eventBus) {
	const auto& [entity, matb] = ctx.renderQueue->skybox.front();

	constexpr TextureBinding textureBindings[] = {
		{"skybox", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *matb.shader);
}

void SkyboxPass::execute(const RenderContext& ctx) {
	const auto& [entity, matb] = ctx.renderQueue->skybox.front();
	const auto [materialIdx, textureOffset, textureCount, shader, meshes] = matb;
	const uint32_t meshIdx = meshes.front();
	const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
	const uint32_t textureId = ctx.renderQueue->material.textures[textureOffset];

	ctx.sceneBuffer->bind();
	shader->activate();
	shader->setMat4("skyView", ctx.camera.skyView);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	// Draw
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}
