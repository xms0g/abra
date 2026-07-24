#include "bloom.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../graph.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Bloom::Bloom(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	mBrightFilter = RESOURCE_MANAGER_INSTANCE.get<Shader>("bloomBF");
	mBlur = RESOURCE_MANAGER_INSTANCE.get<Shader>("bloomBlur");
	mCombine = RESOURCE_MANAGER_INSTANCE.get<Shader>("bloomCombine");
}

Bloom::~Bloom() = default;

void Bloom::configure(const FrameGraph& graph) {
	constexpr TextureBinding textureBindings[] = {
		{.name = "screenTexture", .slot = 0},
	};

	constexpr TextureBinding combineTextureBindings[] = {
		{.name = "screenTexture", .slot = 0},
		{.name = "bloomBlur", .slot = 1}
	};

	RenderCommand::setTextureUnits(textureBindings, *mBrightFilter);
	RenderCommand::setTextureUnits(textureBindings, *mBlur);
	RenderCommand::setTextureUnits(combineTextureBindings, *mCombine);

	mRenderTargets = {&graph.getResource("bloomPing"), &graph.getResource("bloomPong")};
}

uint32_t Bloom::render(const uint32_t vao, const uint32_t sceneTexture, FrameBuffer* renderTarget) const {
	(void) renderTarget;
	bool toggle = false;
	uint32_t inputTex = sceneTexture;

	inputTex = brightFilterPass(vao, inputTex, toggle);
	inputTex = blurPass(vao, inputTex, toggle);
	inputTex = combinePass(vao, sceneTexture, inputTex, toggle);

	return inputTex;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

uint32_t Bloom::brightFilterPass(const uint32_t vao, const uint32_t sceneTexture, bool& toggle) const {
	mRenderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mBrightFilter->bind();

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

		mBlur->bind();
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
	mCombine->bind();

	const uint32_t textures[] = {sceneTexture, blurTexture};
	RenderCommand::drawQuad(vao, textures);

	return mRenderTargets[toggle]->texture();
}
