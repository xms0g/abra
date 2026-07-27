#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../graphicsEncoder.h"
#include "../../graphicsPipeline.h"

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

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus);

private:
	void directionalShadowPass();

	void omnidirectionalShadowPass();

	void perspectiveShadowPass();

	void onGuiUpdate(const UpdateShadowMapEvent& event);

	struct alignas(16) ShadowData {
		glm::mat4 lightSpaceMatrix{};
		glm::mat4 persLightSpaceMatrix[4]{};
		glm::vec4 omniFarPlane{};
	};

	ShadowData mGPUData;

	int32_t mWidth{0}, mHeight{0};
	const RenderContext* mCtx{nullptr};
	const FrameGraph* mGraph{nullptr};
	UniformBuffer mUBO;
	GraphicsPipeline mPipelines[2];
	GraphicsEncoder mEncoder;
	std::unique_ptr<DirectionalShadow> mDirShadow;
	std::unique_ptr<OmnidirectionalShadow> mOmnidirShadow;
	std::unique_ptr<PerspectiveShadow> mPersShadow;
};
