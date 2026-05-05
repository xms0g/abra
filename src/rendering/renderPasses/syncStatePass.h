#pragma once
#include <cstdint>
#include <cstddef>
#include "IRenderPass.hpp"

struct GuiDebugEvent;

class SyncStatePass final : public IRenderPass {
public:
	~SyncStatePass() override;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void syncDebugState(const RenderContext& ctx) const;

	void onDebugUpdate(const GuiDebugEvent& event);

	size_t mEntityID{0};
	uint32_t mDebugMode{0};
};
