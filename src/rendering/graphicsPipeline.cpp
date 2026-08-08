#include "graphicsPipeline.h"

GraphicsPipeline::GraphicsPipeline(PipelineRenderingInfo& renderingInfo) {
	mState.primitiveAssemblyState = renderingInfo.primitiveAssemblyState;
	mState.rasterizationState = renderingInfo.rasterizationState;
	mState.multisampleState = renderingInfo.multisampleState;
	mState.depthStencilState = renderingInfo.depthStencilState;
	mState.colorBlendState = renderingInfo.colorBlendState;
	mState.tessellationState = renderingInfo.tessellationState;

	for (const auto& info: renderingInfo.stages) {
		ShaderStage stage{info};
		mState.shader.attachStage(stage);
	}
	mState.shader.link();

	mState.shader.bind();
	for (const auto& [name, type, binding]: renderingInfo.descriptors) {
		switch (type) {
			case DescriptorType::UniformBuffer: {
				const uint32_t index = glGetUniformBlockIndex(mState.shader.id(), name.c_str());
				glUniformBlockBinding(mState.shader.id(), index, binding);
				break;
			}
			case DescriptorType::Sampler2D:
			case DescriptorType::SamplerCube:
			case DescriptorType::Sampler2DArray:
			case DescriptorType::SamplerCubeArray:
				mState.shader.setValue(name, binding);
				break;
		}
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

PipelinePrimitiveAssemblyState& GraphicsPipeline::primitiveAssemblyState() {
	return mState.primitiveAssemblyState;
}

PipelineRasterizationState& GraphicsPipeline::rasterizationState() {
	return mState.rasterizationState;
}

PipelineMultisampleState& GraphicsPipeline::multisampleState() {
	return mState.multisampleState;
}

PipelineDepthStencilState& GraphicsPipeline::depthStencilState() {
	return mState.depthStencilState;
}

PipelineColorBlendState& GraphicsPipeline::colorBlendState() {
	return mState.colorBlendState;
}

PipelineTessellationState& GraphicsPipeline::tessellationState() {
	return mState.tessellationState;
}

uint32_t GraphicsPipeline::program() const {
	return mState.shader.id();
}

void GraphicsPipeline::bind() const {
	mState.shader.bind();
}

GraphicsPipeline GraphicsPipeline::createFullscreenQuadPipeline(std::vector<PipelineShaderStage> stages,
                                                                std::vector<DescriptorBinding> resources) {
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
		.depthCompareOp = CompareOp::Never,
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
		.descriptors = std::move(resources),
	};

	return GraphicsPipeline{pipelineInfo};
}
