#include "graphicsPipeline.h"

GraphicsPipeline::GraphicsPipeline(PipelineRenderingInfo& renderingInfo) {
	mState.primitiveAssemblyState = renderingInfo.primitiveAssemblyState;
	mState.rasterizationState = renderingInfo.rasterizationState;
	mState.depthStencilState = renderingInfo.depthStencilState;
	mState.colorBlendState = renderingInfo.colorBlendState;
	mState.tessellationState = renderingInfo.tessellationState;

	for (const auto& info: renderingInfo.stages) {
		ShaderStage stage{info};
		mState.shader.attachStage(stage);
	}
	mState.shader.link();

	mState.shader.bind();
	for (const auto& [name, slot]: renderingInfo.samplers) {
		mState.shader.setValue(name, slot);
	}

	for (const auto& [name, binding]: renderingInfo.uniforms) {
		const uint32_t ubidx = glGetUniformBlockIndex(mState.shader.id(), name.c_str());
		glUniformBlockBinding(mState.shader.id(), ubidx, binding);
	}
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept
	: mState(std::move(other.mState)) {
}

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept {
	if (this != &other) {
		mState = std::move(other.mState);
	}

	return *this;
}

PipelineState& GraphicsPipeline::state() {
	return mState;
}

GraphicsPipeline GraphicsPipeline::createFullscreenQuadPipeline(
	std::vector<PipelineShaderStage> stages,
	std::vector<SamplerInfo> samplers) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = std::move(stages),
		.samplers = std::move(samplers),
		.uniforms = {}
	};

	return GraphicsPipeline{pipelineInfo};
}
