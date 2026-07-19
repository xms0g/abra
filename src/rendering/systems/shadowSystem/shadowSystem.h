#pragma once
#include <memory>
#include <array>
#include "glm/glm.hpp"
#include "../../buffers/uniformBuffer.h"

class RenderGraph;
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

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus);

private:
	void directionalShadowPass();

	void omnidirectionalShadowPass() const;

	void perspectiveShadowPass();

	void onGuiUpdate(const UpdateShadowMapEvent& event);

	int32_t mWidth{0};
	int32_t mHeight{0};
	const RenderContext* mCtx{nullptr};
	const RenderGraph* mGraph{nullptr};
	UniformBuffer mUBO;
	std::unique_ptr<DirectionalShadow> mDirShadow;
	std::unique_ptr<OmnidirectionalShadow> mOmnidirShadow;
	std::unique_ptr<PerspectiveShadow> mPersShadow;

	struct alignas(16) ShadowData {
		glm::mat4 lightSpaceMatrix{};
		glm::mat4 persLightSpaceMatrix[4]{};
		glm::vec4 omniFarPlane{};
	};

	ShadowData mGPUData;
};
