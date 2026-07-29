#include "graphicsPipeline.h"
#include "shader.h"
#include "../resource/resourceManager.h"

GraphicsPipeline::GraphicsPipeline(PipelineRenderingInfo& renderingInfo) {
	mState.inputAssembly = renderingInfo.primitiveAssembly;
	mState.rasterization = renderingInfo.rasterization;
	mState.depthStencil = renderingInfo.depthStencil;
	mState.colorBlend = renderingInfo.colorBlend;
	mState.tessellation = renderingInfo.tessellation;

	for (const auto& stage: renderingInfo.stages) {
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

GraphicsPipeline GraphicsPipeline::createFullscreenQuadPipeline(std::vector<ShaderStage>& stages, const std::vector<SamplerInfo>& samplers) {
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
		.stages = std::move(stages),
		.samplers = samplers,
		.uniforms = {}
	};

	return GraphicsPipeline{pipelineInfo};
}
