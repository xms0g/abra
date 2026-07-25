#include "graphicsEncoder.h"
#include "glad/glad.h"
#include "graph.h"
#include "graphicsPipeline.h"
#include "shader.h"
#include "buffers/frameBuffer.h"
#include "material/material.hpp"
#include "../resource/resourceManager.h"

static constexpr uint32_t toGL(const ClearMask mask) {
	return toUnderlying(mask);
}

static constexpr uint32_t toGL(const BlitMask mask) {
	return toUnderlying(mask);
}

static constexpr uint32_t toGL(const BlendFactor factor) {
	return toUnderlying(factor);
}

static constexpr uint32_t toGL(const BlendOp op) {
	return toUnderlying(op);
}

static constexpr uint32_t toGL(const CompareOp op) {
	return toUnderlying(op);
}

static constexpr uint32_t toGL(const CullMode mode) {
	return toUnderlying(mode);
}

static constexpr uint32_t toGL(const FrontFace face) {
	return toUnderlying(face);
}

static constexpr uint32_t toGL(const PrimitiveTopology topology) {
	return toUnderlying(topology);
}

static constexpr uint32_t toGL(const PolygonMode mode) {
	return toUnderlying(mode);
}

static constexpr uint32_t toGL(const PolygonFace face) {
	return toUnderlying(face);
}

GraphicsEncoder::GraphicsEncoder(const FrameGraph& graph) {
	mState.graph = &graph;
}

void GraphicsEncoder::bindFrameBuffer(const std::string& name) const {
	mState.graph->getResource(name).bind();
}

void GraphicsEncoder::bindTexture(const std::string& name, const uint32_t slot, const uint32_t idx) const {
	try {
		mState.graph->getResource(name).bindTexture(slot, idx);
	} catch (...) {
		RESOURCE_MANAGER_INSTANCE.get<BaseFrameBuffer>(name)->bindTexture(slot, idx);
	}
}

void GraphicsEncoder::bindPipeline(GraphicsPipeline& pipeline) {
	mState.pipeline = &pipeline;

	const auto& pipelineState = mState.pipeline->state();

	if (pipelineState.depthStencil.depthTestEnable) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(toGL(pipelineState.depthStencil.depthCompareOp));
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	glDepthMask(pipelineState.depthStencil.depthWriteEnable ? GL_TRUE : GL_FALSE);

	if (pipelineState.rasterization.cullMode != CullMode::None) {
		glEnable(GL_CULL_FACE);
		glCullFace(toGL(pipelineState.rasterization.cullMode));
		glFrontFace(toGL(pipelineState.rasterization.frontFace));
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (pipelineState.colorBlend.blendEnable) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(
			toGL(pipelineState.colorBlend.srcColorBlendFactor),
			toGL(pipelineState.colorBlend.dstColorBlendFactor),
			toGL(pipelineState.colorBlend.srcAlphaBlendFactor),
			toGL(pipelineState.colorBlend.dstAlphaBlendFactor));
		glBlendEquationSeparate(
			toGL(pipelineState.colorBlend.colorBlendOp),
			toGL(pipelineState.colorBlend.alphaBlendOp));

		glColorMask(
			static_cast<bool>(pipelineState.colorBlend.colorWriteMask & ColorComponent::Red),
			static_cast<bool>(pipelineState.colorBlend.colorWriteMask & ColorComponent::Green),
			static_cast<bool>(pipelineState.colorBlend.colorWriteMask & ColorComponent::Blue),
			static_cast<bool>(pipelineState.colorBlend.colorWriteMask & ColorComponent::Alpha)
		);
	} else {
		glDisable(GL_BLEND);
	}

	if (pipelineState.tessellation.patchControlPoints > 0) {
		glPatchParameteri(GL_PATCH_VERTICES, pipelineState.tessellation.patchControlPoints);
	}

	glPolygonMode(toGL(pipelineState.rasterization.polygonFace), toGL(pipelineState.rasterization.polygonMode));

	pipelineState.stage.bind();
}

void GraphicsEncoder::bindMaterial(const MaterialView& material) {
	if (mState.materialCache.lastMaterialIdx == material.idx) {
		return;
	}

	mState.materialCache.lastMaterialIdx = material.idx;

	auto& pipelineState = mState.pipeline->state();

	if (mState.materialCache.lastMatFlags != material.flags || mState.materialCache.lastShader != &pipelineState.stage) {
		mState.materialCache.lastMatFlags = material.flags;
		mState.materialCache.lastShader = &pipelineState.stage;
		pipelineState.stage.setUint("material.flags", material.flags);
	}

	if (material.flags & HAS_HEIGHT_MAP) {
		pipelineState.stage.setFloat("material.heightScale", material.heightScale);
	}

	if (material.flags & ALPHACUTOFF) {
		pipelineState.stage.setFloat("material.alphaCutoff", material.alphaCutoff);
	}

	if (material.flags & HAS_SOLID_COLOR) [[unlikely]] {
		pipelineState.stage.setVec3("material.color", material.color);
	} else {
		int slot{0};

		for (const auto& texture: material.textures) {
			glActiveTexture(GL_TEXTURE0 + slot++);
			glBindTexture(material.textureTarget, texture);
		}
	}

	if (material.flags & TWOSIDED && pipelineState.rasterization.cullMode != CullMode::None) [[unlikely]] {
		glDisable(GL_CULL_FACE);
		pipelineState.rasterization.cullMode = CullMode::None;
	} else if (!(material.flags & TWOSIDED) && pipelineState.rasterization.cullMode == CullMode::None) {
		glEnable(GL_CULL_FACE);
		pipelineState.rasterization.cullMode = CullMode::Back;
	}
}

void GraphicsEncoder::bindTransform(const TransformView& transform) const {
	mState.pipeline->state().stage.setMat4("model", transform.model);
	mState.pipeline->state().stage.setMat3("normalMatrix", transform.normal);
}

void GraphicsEncoder::blitFramebuffer(const std::string& src, const std::string& dst, const BlitMask mask) const {
	const auto& destBuffer = mState.graph->getResource(dst);
	const auto& sourceBuffer = mState.graph->getResource(src);

	sourceBuffer.bindForRead();
	destBuffer.bindForDraw();

	glBlitFramebuffer(0, 0, sourceBuffer.width(), sourceBuffer.height(),
	                  0, 0, destBuffer.width(), destBuffer.height(),
	                  toGL(mask), GL_NEAREST);
}

void GraphicsEncoder::clearFrameBuffer(const ClearMask mask) const {
	glClear(toGL(mask));
}

void GraphicsEncoder::draw(const MeshView& mesh) const {
	glBindVertexArray(mesh.vao);
	glDrawArrays(toGL(mState.pipeline->state().inputAssembly.topology), 0,
	             static_cast<int32_t>(mesh.vertexCount));
}

void GraphicsEncoder::drawIndexed(const MeshView& mesh) const {
	glBindVertexArray(mesh.vao);
	glDrawElements(toGL(mState.pipeline->state().inputAssembly.topology),
	               static_cast<int32_t>(mesh.indexCount), GL_UNSIGNED_INT, nullptr);
}

void GraphicsEncoder::reset() {
	mState.materialCache.reset();
}
