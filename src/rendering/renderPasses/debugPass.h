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
	std::array<std::shared_ptr<Shader>, 3> mDebugShaders;
};
