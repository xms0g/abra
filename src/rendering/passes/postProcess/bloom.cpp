#include "bloom.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../models/quad.h"

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

TextureHandle Bloom::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureHandle sceneTexture,
	FrameBuffer* renderTarget) {
	(void) renderTarget;
	bool toggle = false;
	TextureHandle inputTex = sceneTexture;

	inputTex = brightFilterPass(quad.vao(), inputTex, toggle);
	inputTex = blurPass(quad.vao(), inputTex, toggle);
	inputTex = combinePass(quad.vao(), sceneTexture, inputTex, toggle);

	return inputTex;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

TextureHandle Bloom::brightFilterPass(const uint32_t vao, const TextureHandle sceneTexture, bool& toggle) const {
	mRenderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mBrightFilter->bind();

	const uint32_t textures[] = {sceneTexture.id};
	RenderCommand::drawQuad(vao, textures);

	const TextureHandle outTex = mRenderTargets[toggle]->texture();
	toggle = !toggle;

	return outTex;
}

TextureHandle Bloom::blurPass(const uint32_t vao, const TextureHandle sceneTexture, bool& toggle) const {
	bool horizontal = true;
	TextureHandle outTex = sceneTexture;

	for (int i = 0; i < 10; ++i) {
		mRenderTargets[toggle]->bind();
		glClear(GL_COLOR_BUFFER_BIT);

		mBlur->bind();
		mBlur->setBool("horizontal", horizontal);
		horizontal = !horizontal;

		const uint32_t textures[] = {outTex.id};
		RenderCommand::drawQuad(vao, textures);

		outTex = mRenderTargets[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

TextureHandle Bloom::combinePass(
	const uint32_t vao,
	const TextureHandle sceneTexture,
	const TextureHandle blurTexture,
	const bool& toggle) const {
	mRenderTargets[toggle]->bind();
	mCombine->bind();

	const uint32_t textures[] = {sceneTexture.id, blurTexture.id};
	RenderCommand::drawQuad(vao, textures);

	return mRenderTargets[toggle]->texture();
}
