#include "bloom.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Bloom::Bloom(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mBrightFilter = ctx.resourceManager->get<Shader>("bloomBF");
	mBrightFilter->activate();
	mBrightFilter->setInt("screenTexture", 0);

	mBlur = ctx.resourceManager->get<Shader>("bloomBlur");
	mBlur->activate();
	mBlur->setInt("screenTexture", 0);

	mCombine = ctx.resourceManager->get<Shader>("bloomCombine");
	mCombine->activate();
	mCombine->setInt("screenTexture", 0);
	mCombine->setInt("bloomBlur", 1);

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

	RenderCommon::drawQuad(vao, sceneTexture);

	const uint32_t outTex = mPingPong[toggle]->texture();
	toggle = !toggle;
	return outTex;
}

uint32_t Bloom::blurPass( const uint32_t vao, const uint32_t sceneTexture, bool& toggle) const {
	bool horizontal = true;
	uint32_t outTex = sceneTexture;

	for (int i = 0; i < 10; ++i) {
		mPingPong[toggle]->bind();
		glClear(GL_COLOR_BUFFER_BIT);

		mBlur->activate();
		mBlur->setBool("horizontal", horizontal);
		horizontal = !horizontal;

		RenderCommon::drawQuad(vao, outTex);

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

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blurTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);

	mPingPong[toggle]->unbind();

	return mPingPong[toggle]->texture();
}
