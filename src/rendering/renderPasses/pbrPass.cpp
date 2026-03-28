#include "pbrPass.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../models/cube.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../texture/texture.h"
#include "../../io/filesystem.hpp"
#include "../../config/config.hpp"

PBRPass::~PBRPass() = default;

void PBRPass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matb]: ctx.renderQueue->pbrGroups) {
		const auto& [material, shader, meshes] = matb;
		shader->activate();
		shader->setInt("irradianceMap", ctx.PBR.irradianceMap.textureSlot);
	}

	mEquirectangularToCube = std::make_unique<Shader>("pbr/cubemap.vert", "pbr/equirectangularToCube.frag");
	mIrradianceConv = std::make_unique<Shader>("pbr/cubemap.vert", "pbr/irradianceConv.frag");

	Models::Cube cube;
	const auto& cubeMesh = cube.meshes()->at(0).front();

	const CubemapBuffer envMap{ctx.PBR.envMap.size};
	const uint32_t envCubemap = texture::generateCubemap(ctx.PBR.envMap.size);

	const uint32_t HDRTexture = texture::loadHDR(fs::path(ASSET_DIR + ctx.PBR.HDRTexture).c_str());

	// set up projection and view matrices for capturing data onto the 6 cubemap face directions
	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::mat4 captureViews[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	constexpr uint32_t faces = 6;
	// convert HDR equirectangular environment map to cubemap equivalent
	mEquirectangularToCube->activate();
	mEquirectangularToCube->setInt("equirectangularMap", 0);
	mEquirectangularToCube->setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HDRTexture);

	envMap.bind();
	for (uint32_t i = 0; i < faces; ++i) {
		envMap.bindFace(envCubemap, i);
		mEquirectangularToCube->setMat4("view", captureViews[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommon::drawMesh(cubeMesh);
	}

	envMap.unbind();
	ctx.PBR.envMap.binding = envCubemap;

	const CubemapBuffer irradianceMap{ctx.PBR.irradianceMap.size};
	mIrradianceMap = texture::generateCubemap(ctx.PBR.irradianceMap.size);

	// solve diffuse integral by convolution to create an irradiance (cube)map.
	mIrradianceConv->activate();
	mIrradianceConv->setInt("environmentMap", 0);
	mIrradianceConv->setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	irradianceMap.bind();
	for (uint32_t i = 0; i < faces; ++i) {
		irradianceMap.bindFace(mIrradianceMap, i);
		mIrradianceConv->setMat4("view", captureViews[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommon::drawMesh(cubeMesh);
	}

	irradianceMap.unbind();
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void PBRPass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(ctx);
	ctx.sceneBuffer->bind();

	glActiveTexture(GL_TEXTURE0 + ctx.PBR.irradianceMap.textureSlot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mIrradianceMap);

	const Material* lastMaterial = nullptr;
	const Shader* lastShader = nullptr;
	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->pbrObjects) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
			lastMaterial = nullptr;
		}

		RenderCommon::setupTransform(*entity, *lastShader);

		if (lastMaterial != material) {
			lastMaterial = material;
			RenderCommon::setupMaterial(*entity, *lastMaterial, *lastShader);
			RenderCommon::bindTextures(lastMaterial->textures, *lastShader);
		}

		RenderCommon::drawMesh(*mesh);
	}
	if (lastMaterial)
		RenderCommon::unbindTextures(lastMaterial->textures);
	ctx.sceneBuffer->unbind();
}
