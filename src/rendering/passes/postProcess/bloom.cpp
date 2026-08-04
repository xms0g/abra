#include "bloom.h"
#include "../../shader.h"
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../mesh/vertexArray.h"
#include "../../models/quad.h"

Bloom::Bloom(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

Bloom::~Bloom() = default;

void Bloom::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages0;
	stages0.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages0.emplace_back(ShaderLoader::load("post-processing/bloom/brightFilter.frag"), ShaderStageType::Fragment);

	mPipelines[0] = GraphicsPipeline::createFullscreenQuadPipeline(
		stages0,
		{{.name = "screenTexture", .slot = 0}});

	std::vector<PipelineShaderStage> stages1;
	stages1.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages1.emplace_back(ShaderLoader::load("post-processing/bloom/blur.frag"), ShaderStageType::Fragment);

	mPipelines[1] = GraphicsPipeline::createFullscreenQuadPipeline(
		stages1,
		{{.name = "screenTexture", .slot = 0}});

	std::vector<PipelineShaderStage> stages2;
	stages2.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages2.emplace_back(ShaderLoader::load("post-processing/bloom/combine.frag"), ShaderStageType::Fragment);

	mPipelines[2] = GraphicsPipeline::createFullscreenQuadPipeline(
		stages2,
		{
			{.name = "screenTexture", .slot = 0},
			{.name = "bloomBlur", .slot = 1}
		});

	mRenderTargets = {&graph.getResource("bloomPing"), &graph.getResource("bloomPong")};
}

TextureView Bloom::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureView sceneTexture,
	FrameBuffer* renderTarget) {
	(void) renderTarget;
	bool toggle = false;
	TextureView inputTex = sceneTexture;

	inputTex = brightFilterPass(encoder, quad, inputTex, toggle);
	inputTex = blurPass(encoder, quad, inputTex, toggle);
	inputTex = combinePass(encoder, quad, sceneTexture, inputTex, toggle);

	return inputTex;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

TextureView Bloom::brightFilterPass(
	GraphicsEncoder& encoder,
	const Model::Quad& quad,
	const TextureView sceneTexture,
	bool& toggle) {
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[0]);

	const TextureView textures[] = {sceneTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.bindVertexArray(quad.vao().id());
	encoder.draw(6);

	const FrameBuffer* renderTarget = mRenderTargets[toggle];
	toggle = !toggle;

	return renderTarget->texture();
}

TextureView Bloom::blurPass(
	GraphicsEncoder& encoder,
	const Model::Quad& quad,
	const TextureView sceneTexture,
	bool& toggle) {

	TextureView outTex = sceneTexture;

	encoder.bindPipeline(mPipelines[1]);
	for (int i = 0; i < 10; ++i) {
		encoder.bindFrameBuffer(*mRenderTargets[toggle]);

		encoder.setUniform("horizontal", (i & 1) == 0);

		const TextureView textures[] = {outTex};
		encoder.bindMaterial({.textures = std::span(textures)});

		encoder.bindVertexArray(quad.vao().id());
		encoder.draw(6);

		outTex = mRenderTargets[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

TextureView Bloom::combinePass(
	GraphicsEncoder& encoder,
	const Model::Quad& quad,
	const TextureView sceneTexture,
	const TextureView blurTexture,
	const bool& toggle) {
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	//encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[2]);

	const TextureView textures[] = {sceneTexture, blurTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.bindVertexArray(quad.vao().id());
	encoder.draw(6);

	return mRenderTargets[toggle]->texture();
}
