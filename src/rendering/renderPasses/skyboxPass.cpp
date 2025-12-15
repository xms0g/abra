#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../mesh/mesh.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../texture/texture.h"

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx) {
	const auto& [entity, mbatch] = ctx.renderQueue->skybox.front();
	mbatch.shader->activate();
	mbatch.shader->setInt("skybox", 0);

	ctx.camera.ubo->configure(mbatch.shader->ID(), 0, "CameraBlock");
}

void SkyboxPass::execute(const RenderContext& ctx) {
	const auto& [entity, mbatch] = ctx.renderQueue->skybox.front();
	const Mesh& mesh = mbatch.meshes->front();
	const Texture& tex = mbatch.material->textures.front();

	ctx.sceneBuffer->bind();
	mbatch.shader->activate();
	mbatch.shader->setMat4("skyView", ctx.camera.skyView);

	glActiveTexture(GL_TEXTURE0); // active proper texture unit before binding
	// and finally bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex.id);
	// Draw
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	mesh.bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	mesh.unbind();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	ctx.sceneBuffer->unbind();
}
