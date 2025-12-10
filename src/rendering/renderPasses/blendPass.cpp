#include "blendPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../ECS/components/bv.hpp"
#include "../../ECS/entity.hpp"

BlendPass::~BlendPass() = default;

void BlendPass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matBatches]: ctx.renderQueue->blendGroups) {
		for (const auto& [material, shader, meshes]: matBatches) {
			shader->activate();
			shader->setInt("shadowMap", ctx.shadowMap.textureSlot);
			shader->setInt("shadowCubemap", ctx.shadowMap.textureSlot + 1);
			shader->setInt("persShadowMap", ctx.shadowMap.textureSlot + 2);
		}
	}
}

void BlendPass::execute(const RenderContext& ctx) {
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ctx.sceneBuffer->bind();
	for (const auto& [entity, matBatches]: ctx.renderQueue->blendGroups) {
		if (!entity->getComponent<BoundingVolumeComponent>().isVisible)
			continue;

		for (const auto& [material, shader, meshes]: matBatches) {
			shader->activate();

			RenderCommon::setupTransform(*entity, *shader);
			RenderCommon::setupMaterial(*entity, *material, *shader);

			RenderCommon::bindTextures(material->textures, *shader);
			RenderCommon::drawMeshes(*meshes);
			RenderCommon::unbindTextures(material->textures);
		}
	}
	ctx.sceneBuffer->unbind();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
