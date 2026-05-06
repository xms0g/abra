#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../mesh/mesh.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../texture/texture.h"

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	const auto& [entity, matb] = ctx.renderQueue->skybox.front();
	matb.shader->activate();
	matb.shader->setInt("skybox", 0);
}

void SkyboxPass::execute(const RenderContext& ctx) {
	const auto& [entity, matb] = ctx.renderQueue->skybox.front();
	const auto [materialIdx, shader, meshes] = matb;
	const Mesh& mesh = meshes->front();
	const uint32_t tex = ctx.renderQueue->matTextures.at(materialIdx).front();

	ctx.sceneBuffer->bind();
	shader->activate();
	shader->setMat4("skyView", ctx.camera.skyView);

	glActiveTexture(GL_TEXTURE0); // active proper texture unit before binding
	// and finally bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, ctx.PBR.envMap.binding != 0 ? ctx.PBR.envMap.binding : tex);
	// Draw
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	mesh.bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	mesh.unbind();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}
