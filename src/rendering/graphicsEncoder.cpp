#include "graphicsEncoder.h"
#include "glad/glad.h"
#include "graphicsPipeline.h"
#include "shader.h"
#include "buffers/frameBuffer.h"

GLu(ClearMask)
GLu(BlitMask)
GLu(BlendFactor)
GLu(BlendOp)
GLu(CompareOp)
GLu(CullMode)
GLu(FrontFace)
GLu(PrimitiveTopology)
GLu(PolygonMode)
GLu(PolygonFace)

void GraphicsEncoder::beginRendering(const RenderingInfo& info) const {
	info.framebuffer.bind();

	ClearMask mask{ClearMask::None};
	mask |= info.clearColor ? ClearMask::Color : ClearMask::None;
	mask |= info.clearDepth ? ClearMask::Depth : ClearMask::None;
	mask |= info.clearStencil ? ClearMask::Stencil : ClearMask::None;

	clearFrameBuffer(mask);
	setViewport(info.viewport);
}

void GraphicsEncoder::bindFrameBuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsEncoder::unbindFrameBuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsEncoder::bindFrameBuffer(const FrameBuffer& fb) const {
	fb.bind();
	setViewport({.x = 0, .y = 0, .width = fb.width(), .height = fb.height()});
}

void GraphicsEncoder::bindVertexArray(const uint32_t vao) const {
	glBindVertexArray(vao);
}

void GraphicsEncoder::bindTexture(const TextureHandle& handle, const uint32_t slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(toGLu(handle.target), handle.id);
}

void GraphicsEncoder::bindTextures(const std::span<const TextureHandle> handles, uint32_t slot) const {
	for (const auto& handle: handles) {
		bindTexture(handle, slot++);
	}
}

void GraphicsEncoder::bindPipeline(GraphicsPipeline& pipeline) {
	mState.pipeline = &pipeline;

	const auto& pipelineState = mState.pipeline->state();

	if (pipelineState.depthStencilState.depthTestEnable) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(toGLu(pipelineState.depthStencilState.depthCompareOp));
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	glDepthMask(pipelineState.depthStencilState.depthWriteEnable ? GL_TRUE : GL_FALSE);

	if (pipelineState.depthStencilState.stencilTestEnable) {
		glEnable(GL_STENCIL_TEST);
	}

	if (pipelineState.rasterizationState.cullMode != CullMode::None) {
		glEnable(GL_CULL_FACE);
		glCullFace(toGLu(pipelineState.rasterizationState.cullMode));
		glFrontFace(toGLu(pipelineState.rasterizationState.frontFace));
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (pipelineState.colorBlendState.blendEnable) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(
			toGLu(pipelineState.colorBlendState.srcColorBlendFactor),
			toGLu(pipelineState.colorBlendState.dstColorBlendFactor),
			toGLu(pipelineState.colorBlendState.srcAlphaBlendFactor),
			toGLu(pipelineState.colorBlendState.dstAlphaBlendFactor));
		glBlendEquationSeparate(
			toGLu(pipelineState.colorBlendState.colorBlendOp),
			toGLu(pipelineState.colorBlendState.alphaBlendOp));

		glColorMask(
			static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Red),
			static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Green),
			static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Blue),
			static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Alpha)
		);
	} else {
		glDisable(GL_BLEND);
	}

	if (pipelineState.tessellationState.patchControlPoints > 0) {
		glPatchParameteri(GL_PATCH_VERTICES, pipelineState.tessellationState.patchControlPoints);
	}

	glPolygonMode(toGLu(pipelineState.rasterizationState.polygonFace),
	              toGLu(pipelineState.rasterizationState.polygonMode));

	pipelineState.shader.bind();
}

void GraphicsEncoder::bindMaterial(const MaterialView& material) {
	if (mState.materialCache.lastMaterialIdx == material.idx) {
		return;
	}

	mState.materialCache.lastMaterialIdx = material.idx;

	auto& pipelineState = mState.pipeline->state();

	if (mState.materialCache.lastMatFlags != material.flags || mState.materialCache.lastShader != &pipelineState.
	    shader) {
		mState.materialCache.lastMatFlags = material.flags;
		mState.materialCache.lastShader = &pipelineState.shader;
		setUniform("material.flags", material.flags);
	}

	if (material.flags & HAS_HEIGHT_MAP) {
		setUniform("material.heightScale", material.heightScale);
	}

	if (material.flags & ALPHACUTOFF) {
		setUniform("material.alphaCutoff", material.alphaCutoff);
	}

	if (material.flags & HAS_SOLID_COLOR) [[unlikely]] {
		setUniform("material.color", material.color);
	} else {
		constexpr int slot{0};
		bindTextures(material.textures, slot);
	}

	if (material.flags & TWOSIDED && pipelineState.rasterizationState.cullMode != CullMode::None) [[unlikely]] {
		glDisable(GL_CULL_FACE);
		pipelineState.rasterizationState.cullMode = CullMode::None;
	} else if (!(material.flags & TWOSIDED) && pipelineState.rasterizationState.cullMode == CullMode::None) {
		glEnable(GL_CULL_FACE);
		pipelineState.rasterizationState.cullMode = CullMode::Back;
	}
}

void GraphicsEncoder::bindTransform(const TransformView& transform) {
	setUniform("model", transform.model);
	setUniform("normalMatrix", transform.normal);
}

void GraphicsEncoder::blitFramebuffer(const FrameBuffer& src, const FrameBuffer& dst, const BlitMask mask) const {
	src.bindForRead();
	dst.bindForDraw();

	glBlitFramebuffer(0, 0, src.width(), src.height(),
	                  0, 0, dst.width(), dst.height(),
	                  toGLu(mask), GL_NEAREST);
}

void GraphicsEncoder::clearFrameBuffer(const ClearMask mask) const {
	glClear(toGLu(mask));
}

void GraphicsEncoder::draw(const size_t vertexCount) const {
	glDrawArrays(toGLu(mState.pipeline->state().primitiveAssemblyState.topology), 0,
	             static_cast<int32_t>(vertexCount));
}

void GraphicsEncoder::drawIndexed(const size_t indexCount) const {
	glDrawElements(toGLu(mState.pipeline->state().primitiveAssemblyState.topology),
	               static_cast<int32_t>(indexCount), GL_UNSIGNED_INT, nullptr);
}

void GraphicsEncoder::drawInstanced(const size_t indexCount, const uint32_t count) const {
	glDrawElementsInstanced(
		toGLu(mState.pipeline->state().primitiveAssemblyState.topology),
		static_cast<int32_t>(indexCount),
		GL_UNSIGNED_INT,
		nullptr,
		static_cast<int32_t>(count));
}

void GraphicsEncoder::setViewport(const Viewport viewport) const {
	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

void GraphicsEncoder::setCullMode(const CullMode mode) {
	glCullFace(toGLu(mode));
}

void GraphicsEncoder::reset() {
	mState.materialCache.reset();
}
