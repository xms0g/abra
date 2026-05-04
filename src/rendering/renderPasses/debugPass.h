#pragma once
#include <array>
#include <memory>
#include "IRenderPass.hpp"

struct GuiDebugEvent;
class Shader;

class DebugPass final : public IRenderPass {
public:
	~DebugPass() override;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void onGuiUpdate(const GuiDebugEvent& event);

	size_t mUpdatedID{0};
	uint32_t mDebugMode{0};
	std::array<std::shared_ptr<Shader>, 3> mDebugShaders;
};
