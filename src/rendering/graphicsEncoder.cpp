#include "graphicsEncoder.h"
#include "glad/glad.h"
#include "graphicsPipeline.h"
#include "shader.h"
#include "buffers/frameBuffer.h"

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

static constexpr uint32_t toGL(const Attachment attachment) {
	return toUnderlying(attachment);
}

void GraphicsEncoder::bindFrameBuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsEncoder::unbindFrameBuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsEncoder::bindFrameBuffer(const BaseFrameBuffer& fb) const {
	fb.bind();
}

void GraphicsEncoder::bindTexture(const TextureHandle handle, const uint32_t slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(toGL(handle.target), handle.id);
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

	if (mState.materialCache.lastMatFlags != material.flags || mState.materialCache.lastShader != &pipelineState.
	    stage) {
		mState.materialCache.lastMatFlags = material.flags;
		mState.materialCache.lastShader = &pipelineState.stage;
		pipelineState.stage.setUint("material.flags", material.flags);
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

void GraphicsEncoder::bindTransform(const TransformView& transform) {
	setUniform("model", transform.model);
	setUniform("normalMatrix", transform.normal);
}

void GraphicsEncoder::blitFramebuffer(const FrameBuffer& src, const FrameBuffer& dst, const BlitMask mask) const {
	src.bindForRead();
	dst.bindForDraw();

	glBlitFramebuffer(0, 0, src.width(), src.height(),
	                  0, 0, dst.width(), dst.height(),
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

void GraphicsEncoder::drawInstanced(const MeshView& mesh, const uint32_t count) const {
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(
		toGL(mState.pipeline->state().inputAssembly.topology),
		static_cast<int32_t>(mesh.indexCount),
		GL_UNSIGNED_INT,
		nullptr,
		static_cast<int32_t>(count));
}

void GraphicsEncoder::setViewport(const int32_t x, const int32_t y, const int32_t width, const int32_t height) const {
	glViewport(x, y, width, height);
}

void GraphicsEncoder::setCullMode(const CullMode mode) {
	glCullFace(toGL(mode));
}

void GraphicsEncoder::attachFramebufferTexture(
	const TextureHandle& texture,
	const Attachment attachment,
	const int32_t mip,
	const int32_t layer) {
	switch (texture.target) {
		case TextureTarget::Texture2D:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				toGL(attachment),
				GL_TEXTURE_2D,
				texture.id,
				mip);
			break;
		case TextureTarget::TextureCubeMap:
			assert(layer >= 0 && layer < 6);

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				toGL(attachment),
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
				texture.id,
				mip);
			break;
		case TextureTarget::Texture2DArray:
		case TextureTarget::TextureCubeMapArray:
			glFramebufferTextureLayer(
				GL_FRAMEBUFFER,
				toGL(attachment),
				texture.id,
				mip,
				layer);
			break;
	}
}

void GraphicsEncoder::reset() {
	mState.materialCache.reset();
}
