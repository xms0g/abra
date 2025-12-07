#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../mesh/mesh.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../texture/texture.h"
#include "../../ECS/components/material.hpp"
#include "../../ECS/components/mesh.hpp"
#include "../../ECS/components/shader.hpp"
#include "../../ECS/entity.hpp"

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx) {
	const auto& skybox = ctx.renderQueue->skyboxEntity;
	const auto& shader = skybox->getComponent<ShaderComponent>().shader;
	shader->activate();
	shader->setInt("skybox", 0);

	ctx.camera.ubo->configure(shader->ID(), 0, "CameraBlock");
}

void SkyboxPass::execute(const RenderContext& ctx) {
	const auto& skybox = ctx.renderQueue->skyboxEntity;
	const auto& shader = skybox->getComponent<ShaderComponent>().shader;
	const uint32_t VAO = skybox->getComponent<MeshComponent>().meshes->at(0).front().VAO();
	const uint32_t texID = skybox->getComponent<MaterialComponent>().materials->at(0).textures.front().id;

	ctx.sceneBuffer->bind();
	shader->activate();
	shader->setMat4("skyView", ctx.camera.skyView);

	glActiveTexture(GL_TEXTURE0); // active proper texture unit before binding
	// and finally bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
	// Draw
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	ctx.sceneBuffer->unbind();
}
