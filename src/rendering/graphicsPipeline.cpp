#include "graphicsPipeline.h"
#include "descriptorSet.h"

GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) {
	mState.primitiveAssemblyState = createInfo.rendering.primitiveAssemblyState;
	mState.rasterizationState = createInfo.rendering.rasterizationState;
	mState.multisampleState = createInfo.rendering.multisampleState;
	mState.depthStencilState = createInfo.rendering.depthStencilState;
	mState.colorBlendState = createInfo.rendering.colorBlendState;
	mState.tessellationState = createInfo.rendering.tessellationState;
	mState.layout = createInfo.layout;

	for (const auto& info: createInfo.rendering.stages) {
		ShaderStage stage{info};
		mState.shader.attachStage(stage);
	}
	mState.shader.link();

	mState.shader.bind();
	for (const auto& [bindings]: createInfo.layout.descriptorSets) {
		for (const auto& [name, type, binding]: bindings) {
			switch (type) {
				case DescriptorType::UniformBuffer: {
					const uint32_t index = glGetUniformBlockIndex(mState.shader.id(), name.c_str());
					glUniformBlockBinding(mState.shader.id(), index, binding);
					break;
				}
				case DescriptorType::SampledImage:
					mState.shader.setValue(name, binding);
					break;
			}
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

PipelineLayout& GraphicsPipeline::layout() {
	return mState.layout;
}

uint32_t GraphicsPipeline::program() const {
	return mState.shader.id();
}

void GraphicsPipeline::bind() const {
	mState.shader.bind();
}

GraphicsPipeline GraphicsPipeline::createFullscreenQuadPipeline(std::vector<PipelineShaderStage> stages,
                                                                const DescriptorSetLayout& layout) {
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
		.stages = std::move(stages)
	};

	PipelineLayout layoutInfo = {.descriptorSets = {layout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = pipelineInfo, .layout = layoutInfo};

	return GraphicsPipeline{createInfo};
}
