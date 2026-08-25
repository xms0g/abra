#include "shadowSystem.hpp"
#include "directionalShadow.hpp"
#include "omnidirectionalShadow.hpp"
#include "perspectiveShadow.hpp"
#include "../../frameGraph.hpp"
#include "../../descriptorSet.hpp"
#include "../../graphicsEncoder.hpp"
#include "../../context/renderContext.hpp"
#include "../../buffers/uniformBuffer.hpp"
#include "../../pushConstants/transformPushConstants.hpp"
#include "../../../ECS/components/directionalLight.hpp"
#include "../../../ECS/components/pointLight.hpp"
#include "../../../ECS/components/spotLight.hpp"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/updateShadowMapEvent.hpp"
#include "../../../config/configManager.hpp"

ShadowSystem::ShadowSystem() = default;

ShadowSystem::~ShadowSystem() = default;

void ShadowSystem::configure(const RenderContext& ctx,
                             const FrameGraph& graph,
                             GraphicsEncoder& encoder,
                             EventBus& eventBus) {
	mCtx = &ctx;
	mGraph = &graph;
	mEncoder = &encoder;
	mIndexes.directional = graph.getResourceID("directional");
	mIndexes.omnidirectional = graph.getResourceID("point");
	mIndexes.perspective = graph.getResourceID("spot");

	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Front,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo depthInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("depth/depth.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("depth/depth.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	PipelineRenderingInfo perDepthInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
				{.code = ShaderLoader::load("depth/perDepth.vert"), .stage = ShaderStageType::Vertex},
				{.code = ShaderLoader::load("depth/depth.frag"), .stage = ShaderStageType::Fragment},
			}
	};

	PipelineRenderingInfo cubeDepthInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("depth/depthCubemap.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("depth/depthCubemap.frag"), .stage = ShaderStageType::Fragment},
			{.code = ShaderLoader::load("depth/depthCubemap.geom"), .stage = ShaderStageType::Geometry}
		},
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
				{
					.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
					.type = DescriptorType::UniformBuffer,
					.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
				},
				{.name = CONFIG_MANAGER.get<std::string>("shadow.ubo.blockName"),
					.type = DescriptorType::UniformBuffer,
					.binding = CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"),
				}
		}
	};

	PushConstantLayout transformPushConstantsLayout = {
		.constants = TransformPushConstants::layout.constants,
		.count = TransformPushConstants::layout.count,
		.baseOffset = 0
	};

	PipelineLayout pipelineLayout = {
		.descriptorSets = {bufferLayout},
		.pushConstants = {transformPushConstantsLayout}
	};

	GraphicsPipelineCreateInfo depthCreateInfo = {.rendering = depthInfo, .layout = pipelineLayout};
	GraphicsPipelineCreateInfo cubeDepthCreateInfo = {.rendering = cubeDepthInfo, .layout = pipelineLayout};
	GraphicsPipelineCreateInfo perDepthCreateInfo = {.rendering = perDepthInfo, .layout = pipelineLayout};

	mPipelines[0] = GraphicsPipeline{depthCreateInfo};
	mPipelines[1] = GraphicsPipeline{cubeDepthCreateInfo};
	mPipelines[2] = GraphicsPipeline{perDepthCreateInfo};

	mDirShadow = std::make_unique<DirectionalShadow>(ctx);
	mOmnidirShadow = std::make_unique<OmnidirectionalShadow>(ctx);
	mPersShadow = std::make_unique<PerspectiveShadow>(ctx);

	mUBO = UniformBuffer{
		sizeof(DirectionalShadowData)
		+ sizeof(OmnidirectionalShadowData)
		+ sizeof(PerspectiveShadowData),
		BufferUsage::Dynamic
	};

	const DescriptorSetLayout layout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"),
			}
		}
	};

	DescriptorSet shadowSet{};
	shadowSet.write(CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"), mUBO);

	encoder.bindDescriptorSet(layout, shadowSet);

	eventBus.subscribeToEvent<ShadowSystem, UpdateShadowMapEvent>(this, &ShadowSystem::onGuiUpdate);

	constexpr UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass() {
	const auto lights = mCtx->light.dirLights;

	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource(mIndexes.directional);
	mEncoder->bindFrameBuffer(frameBuffer);
	mEncoder->setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});
	mEncoder->clearFrameBuffer(ClearMask::Depth);

	mDirShadow->render(*mCtx, *mEncoder, mPipelines[0], mUBO, lights[0]->direction);
}

void ShadowSystem::omnidirectionalShadowPass() {
	const auto lights = mCtx->light.pointLights;

	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource(mIndexes.omnidirectional);
	mEncoder->bindFrameBuffer(frameBuffer);
	mEncoder->setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});
	mEncoder->clearFrameBuffer(ClearMask::Depth);

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mOmnidirShadow->render(*mCtx, *mEncoder, mPipelines[1], mUBO, light->position, i);
	}
	mEncoder->unbindFrameBuffer();
}

void ShadowSystem::perspectiveShadowPass() {
	const auto lights = mCtx->light.spotLights;
	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource(mIndexes.perspective);
	mEncoder->bindFrameBuffer(frameBuffer);
	mEncoder->setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mPersShadow->render(
			*mCtx,
			*mEncoder,
			mPipelines[2],
			frameBuffer,
			mUBO,
			light->direction,
			light->position,
			light->outerCutOff,
			i);
	}

	mEncoder->unbindFrameBuffer();
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	directionalShadowPass();
	omnidirectionalShadowPass();
	perspectiveShadowPass();

	mEncoder->setCullFace(CullMode::Back);
}
