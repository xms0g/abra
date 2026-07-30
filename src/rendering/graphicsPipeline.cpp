#include "graphicsPipeline.h"

GraphicsPipeline::GraphicsPipeline(PipelineRenderingInfo& renderingInfo) {
	mState.inputAssembly = renderingInfo.primitiveAssemblyState;
	mState.rasterization = renderingInfo.rasterizationState;
	mState.depthStencil = renderingInfo.depthStencilState;
	mState.colorBlend = renderingInfo.colorBlendState;
	mState.tessellation = renderingInfo.tessellationState;

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

PipelineState& GraphicsPipeline::state() {
	return mState;
}

GraphicsPipeline GraphicsPipeline::createFullscreenQuadPipeline(std::vector<PipelineShaderStage>& stages, const std::vector<SamplerInfo>& samplers) {
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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = std::move(stages),
		.samplers = samplers,
		.uniforms = {}
	};

	return GraphicsPipeline{pipelineInfo};
}
