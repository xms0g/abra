#include "pbrPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../texture/texture.h"
#include "../../io/filesystem.hpp"
#include "../../config/config.hpp"

PBRPass::~PBRPass() = default;

void PBRPass::configure(const RenderContext& ctx) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	for (const auto& [entity, matb]: ctx.renderQueue->pbrGroups) {
		const auto& [material, shader, meshes] = matb;
		shader->activate();
		shader->setInt("irradianceMap", ctx.PBR.irradianceMap.textureSlot);
		shader->setInt("prefilterMap", ctx.PBR.prefilterMap.textureSlot);
		shader->setInt("brdfLUT", ctx.PBR.brdfLUT.textureSlot);
	}

	mEnvMapBuffer = std::make_unique<CubemapBuffer>(ctx.PBR.envMap.size, true);
	mEnvMapBuffer->checkStatus();

	mIrradianceMapBuffer = std::make_unique<CubemapBuffer>(ctx.PBR.irradianceMap.size);
	mIrradianceMapBuffer->checkStatus();

	mPrefilterMapBuffer = std::make_unique<CubemapBuffer>(ctx.PBR.prefilterMap.size, true, true);
	mPrefilterMapBuffer->checkStatus();

	mBrdfLUTBuffer = std::make_unique<FrameBuffer>(ctx.PBR.brdfLUT.size, ctx.PBR.brdfLUT.size);
	mBrdfLUTBuffer->withTextureFP(16, 2)
			.checkStatus();

	createEnvMap(ctx);
	createIrradianceMap(ctx);
	createPrefilterMap(ctx);
	createBrdfLUT(ctx);

	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void PBRPass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(ctx);
	ctx.sceneBuffer->bind();

	glActiveTexture(GL_TEXTURE0 + ctx.PBR.irradianceMap.textureSlot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mIrradianceMapBuffer->texture());

	glActiveTexture(GL_TEXTURE0 + ctx.PBR.prefilterMap.textureSlot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mPrefilterMapBuffer->texture());

	glActiveTexture(GL_TEXTURE0 + ctx.PBR.brdfLUT.textureSlot);
	glBindTexture(GL_TEXTURE_2D, mBrdfLUTBuffer->texture());

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

void PBRPass::createEnvMap(const RenderContext& ctx) {
	const auto equirectangularToCube = Shader{"pbr/cubemap.vert", "pbr/equirectangularToCube.frag"};
	const auto& cubeMesh = cube.meshes()->at(0).front();
	const uint32_t HDRTexture = texture::loadHDR(fs::path(ASSET_DIR + ctx.PBR.HDRTexture).c_str());

	// convert HDR equirectangular environment map to cubemap equivalent
	equirectangularToCube.activate();
	equirectangularToCube.setInt("equirectangularMap", 0);
	equirectangularToCube.setMat4("projection", mCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HDRTexture);

	mEnvMapBuffer->bind();
	for (uint32_t i = 0; i < FACES; ++i) {
		mEnvMapBuffer->bindFace(i);
		equirectangularToCube.setMat4("view", mCaptureViews[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommon::drawMesh(cubeMesh);
	}

	mEnvMapBuffer->unbind();

	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvMapBuffer->texture());
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	ctx.PBR.envMap.binding = mEnvMapBuffer->texture();
}

void PBRPass::createIrradianceMap(const RenderContext& ctx) {
	const auto irradianceConv = Shader{"pbr/cubemap.vert", "pbr/irradianceConv.frag"};
	const auto& cubeMesh = cube.meshes()->at(0).front();

	// solve diffuse integral by convolution to create an irradiance (cube)map.
	irradianceConv.activate();
	irradianceConv.setInt("environmentMap", 0);
	irradianceConv.setMat4("projection", mCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvMapBuffer->texture());

	mIrradianceMapBuffer->bind();
	for (uint32_t i = 0; i < FACES; ++i) {
		mIrradianceMapBuffer->bindFace(i);
		irradianceConv.setMat4("view", mCaptureViews[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommon::drawMesh(cubeMesh);
	}

	mIrradianceMapBuffer->unbind();
}

void PBRPass::createPrefilterMap(const RenderContext& ctx) {
	const auto prefilter = Shader{"pbr/cubemap.vert", "pbr/prefilter.frag"};
	const auto& cubeMesh = cube.meshes()->at(0).front();

	// run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	prefilter.activate();
	prefilter.setInt("environmentMap", 0);
	prefilter.setMat4("projection", mCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvMapBuffer->texture());

	constexpr uint32_t mipLevels = 5;

	mPrefilterMapBuffer->bind();

	for (uint32_t i = 0; i < mipLevels; ++i) {
		const auto mipSize = static_cast<int32_t>(ctx.PBR.prefilterMap.size * std::pow(0.5, i));

		glBindRenderbuffer(GL_RENDERBUFFER, mPrefilterMapBuffer->rbo());
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
		glViewport(0, 0, mipSize, mipSize);

		const float roughness = static_cast<float>(i) / static_cast<float>(mipLevels - 1);
		prefilter.setFloat("roughness", roughness);

		for (uint32_t j = 0; j < FACES; ++j) {
			mPrefilterMapBuffer->bindFace(j, i);
			prefilter.setMat4("view", mCaptureViews[j]);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCommon::drawMesh(cubeMesh);
		}
	}

	mPrefilterMapBuffer->unbind();
}

void PBRPass::createBrdfLUT(const RenderContext& ctx) const {
	const Models::SingleQuad quad;
	const auto brdfLUT = Shader{"pbr/brdfLUT.vert", "pbr/brdfLUT.frag"};
	// generate a 2D LUT from the BRDF equations used.
	mBrdfLUTBuffer->bind();
	brdfLUT.activate();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(quad.VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	mBrdfLUTBuffer->unbind();
}
