#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../mesh/mesh.h"
#include "../mesh/vertex.hpp"
#include "../mesh/vertexArray.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"
#include "../texture/texture.h"
#include "../../io/filesystem.hpp"
#include "../../config/config.hpp"


DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	mQuad = std::make_unique<Models::SingleQuad>();
	mShader = std::make_unique<Shader>("models/quad.vert", "deferred/lighting.frag");
	mShader->activate();
	mShader->setInt("gPosition", 0);
	mShader->setInt("gNormal", 1);
	mShader->setInt("gAlbedo", 2);
	mShader->setInt("gORM", 3);
	mShader->setInt("ssao", ctx.ssao.textureSlot);
	mShader->setInt("shadowMap", ctx.shadow.textureSlot);
	mShader->setInt("shadowCubemap", ctx.shadow.textureSlot + 1);
	mShader->setInt("persShadowMap", ctx.shadow.textureSlot + 2);
	mShader->setInt("irradianceMap", ctx.PBR.irradianceMap.textureSlot);
	mShader->setInt("prefilterMap", ctx.PBR.prefilterMap.textureSlot);
	mShader->setInt("brdfLUT", ctx.PBR.brdfLUT.textureSlot);

	ctx.camera.ubo.self->configure(mShader->id(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
	ctx.light.ubo.self->configure(mShader->id(), ctx.light.ubo.binding, ctx.light.ubo.blockName);
	ctx.shadow.ubo.self->configure(mShader->id(), ctx.shadow.ubo.binding, ctx.shadow.ubo.blockName);

	mEnvMapBuffer = std::make_unique<CubemapBuffer>(ctx.PBR.envMap.size, true);
	mEnvMapBuffer->checkStatus();

	mIrradianceMapBuffer = std::make_unique<CubemapBuffer>(ctx.PBR.irradianceMap.size);
	mIrradianceMapBuffer->checkStatus();

	mPrefilterMapBuffer = std::make_unique<CubemapBuffer>(ctx.PBR.prefilterMap.size, true, true);
	mPrefilterMapBuffer->checkStatus();

	mBrdfLUTBuffer = std::make_unique<FrameBuffer>(ctx.PBR.brdfLUT.size, ctx.PBR.brdfLUT.size);
	mBrdfLUTBuffer->withTextureFP(GL_RG)
			.checkStatus();

	createEnvMap(ctx);
	createIrradianceMap();
	createPrefilterMap(ctx);
	createBrdfLUT();

	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void DeferredLightingPass::execute(const RenderContext& ctx) {
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	ctx.gBuffer.self->bindForRead();
	ctx.sceneBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, ctx.gBuffer.self->width(), ctx.gBuffer.self->height(),
	                  0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
	                  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ctx.sceneBuffer->bind();
	mShader->activate();

	RenderCommon::bindShadowMaps(ctx);

	ctx.gBuffer.self->bindTexture(0, ctx.gBuffer.positionTextureIdx);
	ctx.gBuffer.self->bindTexture(1, ctx.gBuffer.normalTextureIdx);
	ctx.gBuffer.self->bindTexture(2, ctx.gBuffer.albedoTextureIdx);
	ctx.gBuffer.self->bindTexture(3, ctx.gBuffer.ormTextureIdx);

	ctx.ssao.buffer->bindTexture(ctx.ssao.textureSlot);

	mIrradianceMapBuffer->bindTexture(ctx.PBR.irradianceMap.textureSlot);
	mPrefilterMapBuffer->bindTexture(ctx.PBR.prefilterMap.textureSlot);
	mBrdfLUTBuffer->bindTexture(ctx.PBR.brdfLUT.textureSlot);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

void DeferredLightingPass::createEnvMap(const RenderContext& ctx) {
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
		RenderCommon::drawMesh(cubeMesh.vao().id(), cubeMesh.vertices().size(), cubeMesh.indices().size());
	}

	mEnvMapBuffer->unbind();

	mEnvMapBuffer->bindTexture(0);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	ctx.PBR.envMap.binding = mEnvMapBuffer->texture();
}

void DeferredLightingPass::createIrradianceMap() {
	const auto irradianceConv = Shader{"pbr/cubemap.vert", "pbr/irradianceConv.frag"};
	const auto& cubeMesh = cube.meshes()->at(0).front();

	// solve diffuse integral by convolution to create an irradiance (cube)map.
	irradianceConv.activate();
	irradianceConv.setInt("environmentMap", 0);
	irradianceConv.setMat4("projection", mCaptureProjection);

	mEnvMapBuffer->bindTexture(0);

	mIrradianceMapBuffer->bind();
	for (uint32_t i = 0; i < FACES; ++i) {
		mIrradianceMapBuffer->bindFace(i);
		irradianceConv.setMat4("view", mCaptureViews[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommon::drawMesh(cubeMesh.vao().id(), cubeMesh.vertices().size(), cubeMesh.indices().size());
	}

	mIrradianceMapBuffer->unbind();
}

void DeferredLightingPass::createPrefilterMap(const RenderContext& ctx) {
	const auto prefilter = Shader{"pbr/cubemap.vert", "pbr/prefilter.frag"};
	const auto& cubeMesh = cube.meshes()->at(0).front();

	// run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	prefilter.activate();
	prefilter.setInt("environmentMap", 0);
	prefilter.setMat4("projection", mCaptureProjection);
	prefilter.setFloat("resolution", static_cast<float>(ctx.PBR.envMap.size));

	mEnvMapBuffer->bindTexture(0);

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
			RenderCommon::drawMesh(cubeMesh.vao().id(), cubeMesh.vertices().size(), cubeMesh.indices().size());
		}
	}

	mPrefilterMapBuffer->unbind();
}

void DeferredLightingPass::createBrdfLUT() const {
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

