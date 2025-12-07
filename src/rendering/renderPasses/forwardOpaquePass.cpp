#include "forwardOpaquePass.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../ECS/components/bv.hpp"
#include "../../ECS/entity.hpp"

ForwardOpaquePass::~ForwardOpaquePass() = default;

void ForwardOpaquePass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matBatches]: ctx.renderQueue->forwardOpaqueGroups) {
		for (const auto& [material, shader, meshes]: matBatches) {
			shader->activate();
			shader->setInt("shadowMap", ctx.shadowMap.textureSlot);
			shader->setInt("shadowCubemap", ctx.shadowMap.textureSlot + 1);
			shader->setInt("persShadowMap", ctx.shadowMap.textureSlot + 2);
		}
	}
}

void ForwardOpaquePass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(*ctx.shadowMap.textures);
	ctx.sceneBuffer->bind();

	for (const auto& [entity, matBatches]: ctx.renderQueue->forwardOpaqueGroups) {
		if (!entity->getComponent<BoundingVolumeComponent>().isVisible)
			continue;

		for (const auto& [material, shader, meshes]: matBatches) {
			shader->activate();

			RenderCommon::setupTransform(*entity, *shader);
			RenderCommon::setupMaterial(*entity, *shader);

			RenderCommon::bindTextures(material->textures, *shader);
			RenderCommon::drawMeshes(*meshes);
			RenderCommon::unbindTextures(material->textures);
		}
	}
	ctx.sceneBuffer->unbind();
}


