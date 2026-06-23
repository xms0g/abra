#include "bloom.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"
#include "../../../config/configManager.h"

Bloom::Bloom(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mBrightFilter = rm.get<Shader>("bloomBF");
	mBlur = rm.get<Shader>("bloomBlur");
	mCombine = rm.get<Shader>("bloomCombine");

	const std::vector<TextureBinding> textureBindings = {
		{"screenTexture", 0},
	};

	const std::vector<TextureBinding> combineTextureBindings = {
		{"screenTexture", 0},
		{"bloomBlur", 1}
	};

	RenderCommand::setTextureUnits(textureBindings, *mBrightFilter);
	RenderCommand::setTextureUnits(textureBindings, *mBlur);
	RenderCommand::setTextureUnits(combineTextureBindings, *mCombine);

	for (auto& target: mRenderTargets) {
		target = std::make_unique<FrameBuffer>(
			cfg.get<int32_t>("window.width"),
			cfg.get<int32_t>("window.height"));
#ifdef HDR
		target->withTextureFP(GL_RGBA)
#else
		target->withTexture(GL_RGBA)
#endif
				.checkStatus();
	}
}

Bloom::~Bloom() = default;

uint32_t Bloom::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& renderTargets) const {
	(void) toggle;
	(void) renderTargets;
	bool toggle_ = false;
	uint32_t inputTex = sceneTexture;

	inputTex = brightFilterPass(vao, inputTex, toggle_);
	inputTex = blurPass(vao, inputTex, toggle_);
	inputTex = combinePass(vao, sceneTexture, inputTex, toggle_);

	return inputTex;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

uint32_t Bloom::brightFilterPass(const uint32_t vao, const uint32_t sceneTexture, bool& toggle) const {
	mRenderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mBrightFilter->activate();

	const uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	const uint32_t outTex = mRenderTargets[toggle]->texture();
	toggle = !toggle;
	return outTex;
}

uint32_t Bloom::blurPass(const uint32_t vao, const uint32_t sceneTexture, bool& toggle) const {
	bool horizontal = true;
	uint32_t outTex = sceneTexture;

	for (int i = 0; i < 10; ++i) {
		mRenderTargets[toggle]->bind();
		glClear(GL_COLOR_BUFFER_BIT);

		mBlur->activate();
		mBlur->setBool("horizontal", horizontal);
		horizontal = !horizontal;

		const uint32_t textures[] = {outTex};
		RenderCommand::drawQuad(vao, textures);

		outTex = mRenderTargets[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

uint32_t Bloom::combinePass(
	const uint32_t vao,
	const uint32_t sceneTexture,
	const uint32_t blurTexture,
	const bool& toggle) const {
	mRenderTargets[toggle]->bind();
	mCombine->activate();

	const uint32_t textures[] = {sceneTexture, blurTexture};
	RenderCommand::drawQuad(vao, textures);

	return mRenderTargets[toggle]->texture();
}
