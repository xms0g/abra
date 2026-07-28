#include "graphicsPipeline.h"
#include "shader.h"
#include "../resource/resourceManager.h"

GraphicsPipeline::GraphicsPipeline(PipelineRenderingInfo& renderingInfo) {
	mState.inputAssembly = renderingInfo.primitiveAssembly;
	mState.rasterization = renderingInfo.rasterization;
	mState.depthStencil = renderingInfo.depthStencil;
	mState.colorBlend = renderingInfo.colorBlend;
	mState.tessellation = renderingInfo.tessellation;
	mState.stage = std::move(renderingInfo.stage);

	mState.stage.bind();
	for (const auto& [name, slot]: renderingInfo.samplers) {
		mState.stage.setInt(name, slot);
	}

	for (const auto& [name, binding]: renderingInfo.uniforms) {
		const uint32_t ubidx = glGetUniformBlockIndex(mState.stage.id(), name.c_str());
		glUniformBlockBinding(mState.stage.id(), ubidx, binding);
	}
}

PipelineState& GraphicsPipeline::state() {
	return mState;
}

GraphicsPipeline GraphicsPipeline::createFullscreenQuadPipeline(Shader& shader, const std::vector<SamplerInfo>& samplers) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = false,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo pipelineInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = std::move(shader),
		.samplers = samplers,
		.uniforms = {}
	};

	return GraphicsPipeline{pipelineInfo};
}
