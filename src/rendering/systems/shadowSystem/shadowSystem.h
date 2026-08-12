#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../graphicsPipeline.h"

class GraphicsEncoder;
class FrameGraph;
class EventBus;
struct UpdateShadowMapEvent;
class DirectionalShadow;
class OmnidirectionalShadow;
class PerspectiveShadow;
struct RenderContext;

class ShadowSystem {
public:
	ShadowSystem();

	~ShadowSystem();

	void configure(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder, EventBus& eventBus);

private:
	struct alignas(16) UniformBufferObject {
		glm::mat4 lightSpaceMatrix{};
		glm::mat4 persLightSpaceMatrix[4]{};
		glm::vec4 omniFarPlane{};
	};

	void directionalShadowPass(UniformBufferObject& ubo);

	void omnidirectionalShadowPass();

	void perspectiveShadowPass(UniformBufferObject& ubo);

	void onGuiUpdate(const UpdateShadowMapEvent& event);

	struct ResourceIndexes {
		uint32_t directional;
		uint32_t omnidirectional;
		uint32_t perspective;
	};

	ResourceIndexes mIndexes{};
	const RenderContext* mCtx{nullptr};
	const FrameGraph* mGraph{nullptr};
	GraphicsEncoder* mEncoder{nullptr};
	UniformBuffer mUBO;
	std::array<GraphicsPipeline, 2> mPipelines;
	std::unique_ptr<DirectionalShadow> mDirShadow;
	std::unique_ptr<OmnidirectionalShadow> mOmnidirShadow;
	std::unique_ptr<PerspectiveShadow> mPersShadow;
};
