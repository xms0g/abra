#pragma once
#include <cstdint>
#include <cstddef>
#include "IRenderPass.hpp"
#include "glm/glm.hpp"

struct GuiTransformEvent;
struct GuiDebugEvent;

class SyncStatePass final : public IRenderPass {
public:
	~SyncStatePass() override;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void syncDebugState(const RenderContext& ctx) const;

	void syncTransformState(const RenderContext& ctx) const;

	void onDebugUpdate(const GuiDebugEvent& event);

	void onTransformUpdate(const GuiTransformEvent& event);

	size_t mEntityID{0};
	uint32_t mDebugMode{0};

	struct {
		glm::vec3 position{0.0f};
		glm::vec3 rotation{0.0f};
		glm::vec3 scale{0.0f};
		glm::mat4 model{1.0f};
		glm::mat3 normal{1.0f};
	} transform;
};
