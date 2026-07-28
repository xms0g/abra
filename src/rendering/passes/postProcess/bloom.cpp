#include "bloom.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../models/quad.h"

Bloom::Bloom(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

Bloom::~Bloom() = default;

void Bloom::configure(const FrameGraph& graph) {
	auto shader = Shader{"models/quad.vert", "post-processing/bloom/brightFilter.frag"};
	mPipelines[0] = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{{.name = "screenTexture", .slot = 0}});

	shader = Shader{"models/quad.vert", "post-processing/bloom/blur.frag"};
	mPipelines[1] = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{{.name = "screenTexture", .slot = 0}});

	shader = Shader{"models/quad.vert", "post-processing/bloom/combine.frag"};
	mPipelines[2] = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{
			{.name = "screenTexture", .slot = 0},
			{.name = "bloomBlur", .slot = 1}
		});

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

	inputTex = brightFilterPass(encoder, quad, inputTex, toggle);
	inputTex = blurPass(encoder, quad, inputTex, toggle);
	inputTex = combinePass(encoder, quad, sceneTexture, inputTex, toggle);

	return inputTex;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

TextureHandle Bloom::brightFilterPass(
	GraphicsEncoder& encoder,
	const Model::Quad& quad,
	const TextureHandle sceneTexture,
	bool& toggle) {
	encoder.reset();
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[0]);

	const uint32_t textures[] = {sceneTexture.id};
	encoder.bindMaterial({
		.flags = 0,
		.textureTarget = toGL(TextureTarget::Texture2D),
		.textures = std::span(textures)
	});

	encoder.draw({
		.vao = quad.vao(),
		.vertexCount = 6,
		.indexCount = 0
	});

	const FrameBuffer* renderTarget = mRenderTargets[toggle];
	toggle = !toggle;

	return renderTarget->texture();
}

TextureHandle Bloom::blurPass(
	GraphicsEncoder& encoder,
	const Model::Quad& quad,
	const TextureHandle sceneTexture,
	bool& toggle) {
	bool horizontal = true;

	TextureHandle outTex = sceneTexture;

	encoder.bindPipeline(mPipelines[1]);
	for (int i = 0; i < 10; ++i) {
		encoder.reset();
		encoder.bindFrameBuffer(*mRenderTargets[toggle]);
		encoder.clearFrameBuffer(ClearMask::Color);

		encoder.setUniform("horizontal", horizontal);
		horizontal = !horizontal;

		const uint32_t textures[] = {outTex.id};
		encoder.bindMaterial({
			.flags = 0,
			.textureTarget = toGL(TextureTarget::Texture2D),
			.textures = std::span(textures)
		});

		encoder.draw({
			.vao = quad.vao(),
			.vertexCount = 6,
			.indexCount = 0
		});

		outTex = mRenderTargets[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

TextureHandle Bloom::combinePass(
	GraphicsEncoder& encoder,
	const Model::Quad& quad,
	const TextureHandle sceneTexture,
	const TextureHandle blurTexture,
	const bool& toggle) {
	encoder.reset();
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	//encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[2]);

	const uint32_t textures[] = {sceneTexture.id, blurTexture.id};
	encoder.bindMaterial({
		.flags = 0,
		.textureTarget = toGL(TextureTarget::Texture2D),
		.textures = std::span(textures)
	});

	encoder.draw({
		.vao = quad.vao(),
		.vertexCount = 6,
		.indexCount = 0
	});

	return mRenderTargets[toggle]->texture();
}
