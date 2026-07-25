#include "graphicsPipeline.h"
#include "shader.h"
#include "../resource/resourceManager.h"

GraphicsPipeline::GraphicsPipeline(PipelineRenderingInfo& desc) {
	mState.inputAssembly = desc.primitiveAssembly;
	mState.rasterization = desc.rasterization;
	mState.depthStencil = desc.depthStencil;
	mState.stage = std::move(desc.stage);

	mState.stage.bind();
	for (const auto& [name, slot]: desc.samples) {
		mState.stage.setInt(name, slot);
	}

	for (const auto& [name, binding]: desc.uniforms) {
		const uint32_t ubidx = glGetUniformBlockIndex(mState.stage.id(), name);
		glUniformBlockBinding(mState.stage.id(), ubidx, binding);
	}
}

PipelineState& GraphicsPipeline::state() {
	return mState;
}
