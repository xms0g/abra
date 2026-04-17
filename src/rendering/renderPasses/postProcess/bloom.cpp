#include "bloom.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"

Bloom::Bloom(const std::string& name, const int width, const int height, const bool enabled)
	: IPostEffect(name, enabled) {
	brightFilter = std::make_unique<Shader>("models/quad.vert", "post-processing/bloom/brightFilter.frag");
	brightFilter->activate();
	brightFilter->setInt("screenTexture", 0);

	blur = std::make_unique<Shader>("models/quad.vert", "post-processing/bloom/blur.frag");
	blur->activate();
	blur->setInt("screenTexture", 0);

	combine = std::make_unique<Shader>("models/quad.vert", "post-processing/bloom/combine.frag");
	combine->activate();
	combine->setInt("screenTexture", 0);
	combine->setInt("bloomBlur", 1);

	for (auto& target: mPingPong) {
		target = std::make_unique<FrameBuffer>(width, height);
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
	const uint32_t sceneTexture,
	const uint32_t vao,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	(void) toggle;
	(void) pingPong;
	bool toggle_ = false;
	uint32_t inputTex = sceneTexture;

	inputTex = brightFilterPass(inputTex, vao, toggle_);
	inputTex = blurPass(inputTex, vao, toggle_);
	inputTex = combinePass(sceneTexture, inputTex, vao, toggle_);

	return inputTex;
}

uint32_t Bloom::brightFilterPass(const uint32_t sceneTexture, const uint32_t vao, bool& toggle) const {
	mPingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	brightFilter->activate();

	RenderCommon::drawQuad(sceneTexture, vao);

	const uint32_t outTex = mPingPong[toggle]->texture();
	toggle = !toggle;
	return outTex;
}

uint32_t Bloom::blurPass(const uint32_t sceneTexture, const uint32_t vao, bool& toggle) const {
	bool horizontal = true;
	uint32_t outTex = sceneTexture;

	for (int i = 0; i < 10; i++) {
		mPingPong[toggle]->bind();
		glClear(GL_COLOR_BUFFER_BIT);

		blur->activate();
		blur->setBool("horizontal", horizontal);
		horizontal = !horizontal;

		RenderCommon::drawQuad(outTex, vao);

		outTex = mPingPong[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

uint32_t Bloom::combinePass(
	const uint32_t sceneTexture,
	const uint32_t bloomBlur,
	const uint32_t vao,
	const bool& toggle) const {
	mPingPong[toggle]->bind();
	combine->activate();
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomBlur);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	mPingPong[toggle]->unbind();

	return mPingPong[toggle]->texture();
}
