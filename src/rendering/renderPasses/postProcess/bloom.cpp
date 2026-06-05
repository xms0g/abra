#include "bloom.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Bloom::Bloom(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mBrightFilter = ctx.resourceManager->get<Shader>("bloomBF");
	mBlur = ctx.resourceManager->get<Shader>("bloomBlur");
	mCombine = ctx.resourceManager->get<Shader>("bloomCombine");

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

	for (auto& target: mPingPong) {
		target = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
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
	PingPongBuffer& pingPong) const {
	(void) toggle;
	(void) pingPong;
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
	mPingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mBrightFilter->activate();

	uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	const uint32_t outTex = mPingPong[toggle]->texture();
	toggle = !toggle;
	return outTex;
}

uint32_t Bloom::blurPass(const uint32_t vao, const uint32_t sceneTexture, bool& toggle) const {
	bool horizontal = true;
	uint32_t outTex = sceneTexture;

	for (int i = 0; i < 10; ++i) {
		mPingPong[toggle]->bind();
		glClear(GL_COLOR_BUFFER_BIT);

		mBlur->activate();
		mBlur->setBool("horizontal", horizontal);
		horizontal = !horizontal;

		uint32_t textures[] = {outTex};
		RenderCommand::drawQuad(vao, textures);

		outTex = mPingPong[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

uint32_t Bloom::combinePass(
	const uint32_t vao,
	const uint32_t sceneTexture,
	const uint32_t blurTexture,
	const bool& toggle) const {
	mPingPong[toggle]->bind();
	mCombine->activate();

	uint32_t textures[] = {sceneTexture, blurTexture};
	RenderCommand::drawQuad(vao, textures);

	mPingPong[toggle]->unbind();

	return mPingPong[toggle]->texture();
}
