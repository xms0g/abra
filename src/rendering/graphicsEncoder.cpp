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
	glBindTexture(toGLu(handle.target), handle.id);
}

void GraphicsEncoder::bindPipeline(GraphicsPipeline& pipeline) {
	mState.pipeline = &pipeline;

	const auto& pipelineState = mState.pipeline->state();

	if (pipelineState.depthStencil.depthTestEnable) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(toGLu(pipelineState.depthStencil.depthCompareOp));
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	glDepthMask(pipelineState.depthStencil.depthWriteEnable ? GL_TRUE : GL_FALSE);

	if (pipelineState.depthStencil.stencilTestEnable) {
		glEnable(GL_STENCIL_TEST);
	}

	if (pipelineState.rasterization.cullMode != CullMode::None) {
		glEnable(GL_CULL_FACE);
		glCullFace(toGLu(pipelineState.rasterization.cullMode));
		glFrontFace(toGLu(pipelineState.rasterization.frontFace));
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (pipelineState.colorBlend.blendEnable) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(
			toGLu(pipelineState.colorBlend.srcColorBlendFactor),
			toGLu(pipelineState.colorBlend.dstColorBlendFactor),
			toGLu(pipelineState.colorBlend.srcAlphaBlendFactor),
			toGLu(pipelineState.colorBlend.dstAlphaBlendFactor));
		glBlendEquationSeparate(
			toGLu(pipelineState.colorBlend.colorBlendOp),
			toGLu(pipelineState.colorBlend.alphaBlendOp));

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

	glPolygonMode(toGLu(pipelineState.rasterization.polygonFace), toGLu(pipelineState.rasterization.polygonMode));

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
	                  toGLu(mask), GL_NEAREST);
}

void GraphicsEncoder::clearFrameBuffer(const ClearMask mask) const {
	glClear(toGLu(mask));
}

void GraphicsEncoder::draw(const MeshView& mesh) const {
	glBindVertexArray(mesh.vao);
	glDrawArrays(toGLu(mState.pipeline->state().inputAssembly.topology), 0,
	             static_cast<int32_t>(mesh.vertexCount));
}

void GraphicsEncoder::drawIndexed(const MeshView& mesh) const {
	glBindVertexArray(mesh.vao);
	glDrawElements(toGLu(mState.pipeline->state().inputAssembly.topology),
	               static_cast<int32_t>(mesh.indexCount), GL_UNSIGNED_INT, nullptr);
}

void GraphicsEncoder::drawInstanced(const MeshView& mesh, const uint32_t count) const {
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(
		toGLu(mState.pipeline->state().inputAssembly.topology),
		static_cast<int32_t>(mesh.indexCount),
		GL_UNSIGNED_INT,
		nullptr,
		static_cast<int32_t>(count));
}

void GraphicsEncoder::setViewport(const int32_t x, const int32_t y, const int32_t width, const int32_t height) const {
	glViewport(x, y, width, height);
}

void GraphicsEncoder::setCullMode(const CullMode mode) {
	glCullFace(toGLu(mode));
}

void GraphicsEncoder::reset() {
	mState.materialCache.reset();
}
