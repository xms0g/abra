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
	void syncDebugState(const RenderContext& ctx);

	void syncTransformState(const RenderContext& ctx);

	void onDebugUpdate(const GuiDebugEvent& event);

	void onTransformUpdate(const GuiTransformEvent& event);

	EventBus* mEventBus{nullptr};

	size_t mEntityID{0};
	struct {
		uint32_t mode{0};
		bool isDirty{false};
	} debug;

	struct {
		glm::vec3 position{0.0f};
		glm::vec3 rotation{0.0f};
		glm::vec3 scale{0.0f};
		glm::mat4 model{1.0f};
		glm::mat3 normal{1.0f};
		bool isDirty{false};
	} transform;
};
