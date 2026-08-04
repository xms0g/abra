#include "graphicsEncoder.h"
#include "graphicsPipeline.h"
#include "shader.h"
#include "buffers/frameBuffer.h"

void GraphicsEncoder::beginRendering(const RenderingInfo& info) {
	info.frameBuffer.bind();

	auto mask{ClearMask::None};
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

void GraphicsEncoder::bindFrameBuffer(const FrameBuffer& frameBuffer) {
	if (mState.glStateCache.handles.framebuffer != frameBuffer.id()) {
		mState.glStateCache.handles.framebuffer = frameBuffer.id();
		frameBuffer.bind();
	}
}

void GraphicsEncoder::bindVertexArray(const uint32_t vao) {
	if (mState.glStateCache.handles.vao != vao) {
		mState.glStateCache.handles.vao = vao;
		glBindVertexArray(vao);
	}
}

void GraphicsEncoder::bindTexture(const TextureView& handle, const uint32_t slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(toUnderlying(handle.target), handle.id);
}

void GraphicsEncoder::bindTextures(const std::span<const TextureView> handles, uint32_t slot) {
	for (const auto& handle: handles) {
		if (mState.materialCache.textures[slot] != handle) {
			mState.materialCache.textures[slot] = handle;
			bindTexture(handle, slot);
		}

		++slot;
	}
}

void GraphicsEncoder::bindPipeline(GraphicsPipeline& pipeline) {
	mState.pipeline = &pipeline;

	const auto& pipelineState = mState.pipeline->state();

	if (mState.glStateCache.depthStencil.depthTestEnable != pipelineState.depthStencilState.depthTestEnable) {
		mState.glStateCache.depthStencil.depthTestEnable = pipelineState.depthStencilState.depthTestEnable;

		if (pipelineState.depthStencilState.depthTestEnable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}

	if (mState.glStateCache.depthStencil.depthWriteEnable != pipelineState.depthStencilState.depthWriteEnable) {
		mState.glStateCache.depthStencil.depthWriteEnable = pipelineState.depthStencilState.depthWriteEnable;
		glDepthMask(pipelineState.depthStencilState.depthWriteEnable ? GL_TRUE : GL_FALSE);
	}

	if (mState.glStateCache.depthStencil.depthCompareOp != pipelineState.depthStencilState.depthCompareOp &&
	    pipelineState.depthStencilState.depthCompareOp != CompareOp::Never) {
		mState.glStateCache.depthStencil.depthCompareOp = pipelineState.depthStencilState.depthCompareOp;
		glDepthFunc(toUnderlying(pipelineState.depthStencilState.depthCompareOp));
	}

	if (mState.glStateCache.depthStencil.stencilTestEnable != pipelineState.depthStencilState.stencilTestEnable) {
		mState.glStateCache.depthStencil.stencilTestEnable = pipelineState.depthStencilState.stencilTestEnable;

		if (pipelineState.depthStencilState.stencilTestEnable) {
			glEnable(GL_STENCIL_TEST);
		} else {
			glDisable(GL_STENCIL_TEST);
		}
	}

	if (mState.glStateCache.cull.cullMode != pipelineState.rasterizationState.cullMode) {
		mState.glStateCache.cull.cullMode = pipelineState.rasterizationState.cullMode;

		if (pipelineState.rasterizationState.cullMode != CullMode::None) {
			setCullEnabled(true);
			setCullFace(pipelineState.rasterizationState.cullMode);
		} else {
			setCullEnabled(false);
		}
	}

	if (mState.glStateCache.cull.frontFace != pipelineState.rasterizationState.frontFace) {
		mState.glStateCache.cull.frontFace = pipelineState.rasterizationState.frontFace;
		glFrontFace(toUnderlying(pipelineState.rasterizationState.frontFace));
	}

	if (mState.glStateCache.blend.blendEnable != pipelineState.colorBlendState.blendEnable) {
		mState.glStateCache.blend.blendEnable = pipelineState.colorBlendState.blendEnable;

		if (pipelineState.colorBlendState.blendEnable) {
			glEnable(GL_BLEND);
			glBlendFuncSeparate(
				toUnderlying(pipelineState.colorBlendState.srcColorBlendFactor),
				toUnderlying(pipelineState.colorBlendState.dstColorBlendFactor),
				toUnderlying(pipelineState.colorBlendState.srcAlphaBlendFactor),
				toUnderlying(pipelineState.colorBlendState.dstAlphaBlendFactor));
			glBlendEquationSeparate(
				toUnderlying(pipelineState.colorBlendState.colorBlendOp),
				toUnderlying(pipelineState.colorBlendState.alphaBlendOp));

			glColorMask(
				static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Red),
				static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Green),
				static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Blue),
				static_cast<bool>(pipelineState.colorBlendState.colorWriteMask & ColorComponent::Alpha)
			);
		} else {
			glDisable(GL_BLEND);
		}
	}

	if (pipelineState.tessellationState.patchControlPoints > 0) {
		glPatchParameteri(GL_PATCH_VERTICES, pipelineState.tessellationState.patchControlPoints);
	}

	if (mState.glStateCache.polygon.polygonMode != pipelineState.rasterizationState.polygonMode) {
		mState.glStateCache.polygon.polygonMode = pipelineState.rasterizationState.polygonMode;

		glPolygonMode(
			toUnderlying(pipelineState.rasterizationState.polygonFace),
			toUnderlying(pipelineState.rasterizationState.polygonMode));
	}

	if (mState.glStateCache.handles.program != pipelineState.shader.id()) {
		mState.glStateCache.handles.program = pipelineState.shader.id();

		pipelineState.shader.bind();
	}
}

void GraphicsEncoder::bindMaterial(const MaterialView& material) {
	const auto& pipelineState = mState.pipeline->state();

	if (mState.materialCache.lastMatFlags != material.flags || mState.materialCache.lastShader != pipelineState.shader.id()) {
		mState.materialCache.lastMatFlags = material.flags;
		mState.materialCache.lastShader = pipelineState.shader.id();
		setUniform("material.flags", material.flags);
	}

	if (material.flags & HAS_HEIGHT_MAP) [[unlikely]] {
		setUniform("material.heightScale", material.heightScale);
	}

	if (material.flags & ALPHACUTOFF) [[unlikely]] {
		setUniform("material.alphaCutoff", material.alphaCutoff);
	}

	if (material.flags & HAS_SOLID_COLOR) [[unlikely]] {
		setUniform("material.color", material.color);
	} else {
		constexpr int slot{0};
		bindTextures(material.textures, slot);
	}

	if (material.flags & TWOSIDED &&
	    mState.glStateCache.cull.cullMode != CullMode::None &&
	    pipelineState.rasterizationState.cullMode != CullMode::None) [[unlikely]] {
		mState.glStateCache.cull.cullMode = CullMode::None;
		setCullEnabled(false);
	}

	if (!(material.flags & TWOSIDED) &&
	    mState.glStateCache.cull.cullMode == CullMode::None &&
	    pipelineState.rasterizationState.cullMode != CullMode::None) [[unlikely]] {
		mState.glStateCache.cull.cullMode = pipelineState.rasterizationState.cullMode;
		setCullEnabled(true);
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
	                  toUnderlying(mask), GL_NEAREST);
}

void GraphicsEncoder::clearFrameBuffer(const ClearMask mask) const {
	glClear(toUnderlying(mask));
}

void GraphicsEncoder::draw(const size_t vertexCount) const {
	glDrawArrays(toUnderlying(mState.pipeline->state().primitiveAssemblyState.topology), 0,
	             static_cast<int32_t>(vertexCount));
}

void GraphicsEncoder::drawIndexed(const size_t indexCount) const {
	glDrawElements(toUnderlying(mState.pipeline->state().primitiveAssemblyState.topology),
	               static_cast<int32_t>(indexCount), GL_UNSIGNED_INT, nullptr);
}

void GraphicsEncoder::drawInstanced(const size_t indexCount, const uint32_t count) const {
	glDrawElementsInstanced(
		toUnderlying(mState.pipeline->state().primitiveAssemblyState.topology),
		static_cast<int32_t>(indexCount),
		GL_UNSIGNED_INT,
		nullptr,
		static_cast<int32_t>(count));
}

void GraphicsEncoder::setViewport(const Viewport viewport) {
	if (mState.glStateCache.viewport != viewport) {
		mState.glStateCache.viewport = viewport;

		glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
	}
}

void GraphicsEncoder::setCullEnabled(const bool enabled) {
	if (enabled)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void GraphicsEncoder::setCullFace(const CullMode mode) {
	glCullFace(toUnderlying(mode));
}

void GraphicsEncoder::reset() {
	mState.materialCache.reset();
	mState.glStateCache.reset();
}
