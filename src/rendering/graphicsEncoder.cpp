#include "graphicsEncoder.h"
#include "graphicsPipeline.h"
#include "descriptorSet.h"
#include "buffers/frameBuffer.h"

void GraphicsEncoder::beginRendering(const RenderingInfo& info) {
	glBindFramebuffer(GL_FRAMEBUFFER, info.frameBuffer.fbHandle());

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
	if (mState.glStateCache.handles.framebuffer != frameBuffer.fbHandle()) {
		mState.glStateCache.handles.framebuffer = frameBuffer.fbHandle();
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbHandle());
	}
}

void GraphicsEncoder::bindVertexArray(const uint32_t vao) {
	if (mState.glStateCache.handles.vao != vao) {
		mState.glStateCache.handles.vao = vao;
		glBindVertexArray(vao);
	}
}

void GraphicsEncoder::bindPipeline(GraphicsPipeline& pipeline) {
	mState.pipeline = &pipeline;

	if (mState.glStateCache.depthStencil.depthTestEnable != mState.pipeline->depthStencilState().depthTestEnable) {
		mState.glStateCache.depthStencil.depthTestEnable = mState.pipeline->depthStencilState().depthTestEnable;

		if (mState.pipeline->depthStencilState().depthTestEnable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}

	if (mState.glStateCache.depthStencil.depthWriteEnable != mState.pipeline->depthStencilState().depthWriteEnable) {
		mState.glStateCache.depthStencil.depthWriteEnable = mState.pipeline->depthStencilState().depthWriteEnable;
		glDepthMask(mState.pipeline->depthStencilState().depthWriteEnable ? GL_TRUE : GL_FALSE);
	}

	if (mState.glStateCache.depthStencil.depthCompareOp != mState.pipeline->depthStencilState().depthCompareOp &&
	    mState.pipeline->depthStencilState().depthCompareOp != CompareOp::Never) {
		mState.glStateCache.depthStencil.depthCompareOp = mState.pipeline->depthStencilState().depthCompareOp;
		glDepthFunc(toUnderlying(mState.pipeline->depthStencilState().depthCompareOp));
	}

	if (mState.glStateCache.depthStencil.stencilTestEnable != mState.pipeline->depthStencilState().stencilTestEnable) {
		mState.glStateCache.depthStencil.stencilTestEnable = mState.pipeline->depthStencilState().stencilTestEnable;

		if (mState.pipeline->depthStencilState().stencilTestEnable) {
			glEnable(GL_STENCIL_TEST);
		} else {
			glDisable(GL_STENCIL_TEST);
		}
	}

	if (mState.glStateCache.cull.cullMode != mState.pipeline->rasterizationState().cullMode) {
		mState.glStateCache.cull.cullMode = mState.pipeline->rasterizationState().cullMode;

		setCullMode(mState.pipeline->rasterizationState().cullMode);
	}

	if (mState.glStateCache.cull.frontFace != mState.pipeline->rasterizationState().frontFace) {
		mState.glStateCache.cull.frontFace = mState.pipeline->rasterizationState().frontFace;
		glFrontFace(toUnderlying(mState.pipeline->rasterizationState().frontFace));
	}

	if (mState.glStateCache.aa.samples != mState.pipeline->multisampleState().rasterizationSamples) {
		mState.glStateCache.aa.samples = mState.pipeline->multisampleState().rasterizationSamples;

		if (mState.glStateCache.aa.samples > 0) {
			glEnable(GL_MULTISAMPLE);
		} else {
			glDisable(GL_MULTISAMPLE);
		}
	}

	if (mState.glStateCache.blend.blendEnable != mState.pipeline->colorBlendState().blendEnable) {
		mState.glStateCache.blend.blendEnable = mState.pipeline->colorBlendState().blendEnable;

		if (mState.pipeline->colorBlendState().blendEnable) {
			glEnable(GL_BLEND);
			glBlendFuncSeparate(
				toUnderlying(mState.pipeline->colorBlendState().srcColorBlendFactor),
				toUnderlying(mState.pipeline->colorBlendState().dstColorBlendFactor),
				toUnderlying(mState.pipeline->colorBlendState().srcAlphaBlendFactor),
				toUnderlying(mState.pipeline->colorBlendState().dstAlphaBlendFactor));
			glBlendEquationSeparate(
				toUnderlying(mState.pipeline->colorBlendState().colorBlendOp),
				toUnderlying(mState.pipeline->colorBlendState().alphaBlendOp));

			glColorMask(
				static_cast<bool>(mState.pipeline->colorBlendState().colorWriteMask & ColorComponent::Red),
				static_cast<bool>(mState.pipeline->colorBlendState().colorWriteMask & ColorComponent::Green),
				static_cast<bool>(mState.pipeline->colorBlendState().colorWriteMask & ColorComponent::Blue),
				static_cast<bool>(mState.pipeline->colorBlendState().colorWriteMask & ColorComponent::Alpha)
			);
		} else {
			glDisable(GL_BLEND);
		}
	}

	if (mState.glStateCache.tessellation.patchVertices != mState.pipeline->tessellationState().patchControlPoints) {
		mState.glStateCache.tessellation.patchVertices = mState.pipeline->tessellationState().patchControlPoints;
		glPatchParameteri(GL_PATCH_VERTICES, mState.glStateCache.tessellation.patchVertices);
	}

	if (mState.glStateCache.polygon.polygonMode != mState.pipeline->rasterizationState().polygonMode) {
		mState.glStateCache.polygon.polygonMode = mState.pipeline->rasterizationState().polygonMode;

		glPolygonMode(
			toUnderlying(mState.pipeline->rasterizationState().polygonFace),
			toUnderlying(mState.pipeline->rasterizationState().polygonMode));
	}

	if (mState.glStateCache.handles.program != mState.pipeline->program()) {
		mState.glStateCache.handles.program = mState.pipeline->program();

		mState.pipeline->bind();
	}
}

void GraphicsEncoder::pushConstants(const void* data) const {
	for (int i = 0; i < mState.pipeline->layout().pushConstants.count; ++i) {
		auto& [name, offset, type] = mState.pipeline->layout().pushConstants.constants[i];
		pushConstant(name, static_cast<const std::byte*>(data) + offset, type);
	}
}

void GraphicsEncoder::bindDescriptorSet(const DescriptorSetLayout& layout, const DescriptorSet& descriptorSet) {
	for (int i = 0; i < layout.bindings.size(); ++i) {
		const auto& descBinding = layout.bindings[i];
		auto& descriptor = descriptorSet[i];

		switch (descBinding.type) {
			case DescriptorType::None:
				break;
			case DescriptorType::UniformBuffer: {
				const auto& buffer = std::get<BufferView>(descriptor.resource);

				glBindBufferRange(buffer.target, descBinding.binding, buffer.id, 0, buffer.size);
				break;
			}

			case DescriptorType::SampledImage:{
				const auto& texture = std::get<TextureView>(descriptor.resource);

				if (mState.bindingCache.textures[descBinding.binding] != texture) {
					mState.bindingCache.textures[descBinding.binding] = texture;
					glActiveTexture(GL_TEXTURE0 + descBinding.binding);
					glBindTexture(toUnderlying(texture.target), texture.id);
				}

				break;
			}
		}
	}
}

void GraphicsEncoder::blitFramebuffer(const FrameBuffer& src, const FrameBuffer& dst, const BlitMask mask) const {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src.fbHandle());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.fbHandle());

	glBlitFramebuffer(0, 0, src.width(), src.height(),
	                  0, 0, dst.width(), dst.height(),
	                  toUnderlying(mask), GL_NEAREST);
}

void GraphicsEncoder::resizeRenderBuffer(const FrameBuffer& fb, const int32_t width, const int32_t height) {
	glBindRenderbuffer(GL_RENDERBUFFER, fb.rbHandle());
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	setViewport({.x = 0, .y = 0, .width = width, .height = height});
}

void GraphicsEncoder::attachTexture(const FrameBuffer& fb,
                                    const uint32_t index,
                                    const Attachment attachment,
                                    const int32_t mip,
                                    const int32_t layer) const {
	switch (auto [id, target] = fb.texture(index); target) {
		case TextureTarget::Texture2D:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				toUnderlying(attachment),
				GL_TEXTURE_2D,
				id,
				mip);
			break;
		case TextureTarget::TextureCubeMap:
			assert(layer >= 0 && layer < 6);

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				toUnderlying(attachment),
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
				id,
				mip);
			break;
		case TextureTarget::Texture2DArray:
		case TextureTarget::TextureCubeMapArray:
			glFramebufferTextureLayer(
				GL_FRAMEBUFFER,
				toUnderlying(attachment),
				id,
				mip,
				layer);
			break;
		case TextureTarget::Texture2DMultisample:
			break;
	}
}

void GraphicsEncoder::clearFrameBuffer(const ClearMask mask) const {
	glClear(toUnderlying(mask));
}

void GraphicsEncoder::draw(const size_t vertexCount) const {
	glDrawArrays(toUnderlying(mState.pipeline->primitiveAssemblyState().topology), 0,
	             static_cast<int32_t>(vertexCount));
}

void GraphicsEncoder::drawIndexed(const size_t indexCount) const {
	glDrawElements(toUnderlying(mState.pipeline->primitiveAssemblyState().topology),
	               static_cast<int32_t>(indexCount), GL_UNSIGNED_INT, nullptr);
}

void GraphicsEncoder::drawInstanced(const size_t indexCount, const uint32_t count) const {
	glDrawElementsInstanced(
		toUnderlying(mState.pipeline->primitiveAssemblyState().topology),
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

void GraphicsEncoder::setCullFace(const CullMode mode) {
	glCullFace(toUnderlying(mode));
}

void GraphicsEncoder::setCullMode(const CullMode mode) {
	if (mState.glStateCache.cull.cullMode != mode) {
		mState.glStateCache.cull.cullMode = mode;

		if (mode == CullMode::None)
			glDisable(GL_CULL_FACE);
		else {
			glEnable(GL_CULL_FACE);
			glCullFace(toUnderlying(mode));
		}
	}
}

void GraphicsEncoder::reset() {
	mState.bindingCache.reset();
	mState.glStateCache.reset();
}

void GraphicsEncoder::pushConstant(const std::string_view name, const void* data, const PushConstantType type) const {
	assert(mState.pipeline);
	switch (type) {
		case PushConstantType::Int:
			mState.pipeline->setValue(name, *static_cast<const int*>(data));
			break;
		case PushConstantType::UInt:
			mState.pipeline->setValue(name, *static_cast<const uint32_t*>(data));
			break;
		case PushConstantType::Float:
			mState.pipeline->setValue(name, *static_cast<const float*>(data));
			break;
		case PushConstantType::Vec3:
			mState.pipeline->setValue(name, *static_cast<const glm::vec3*>(data));
			break;
	}
}
