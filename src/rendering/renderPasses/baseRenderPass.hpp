#pragma once
#include <vector>
#include <string>

class RenderGraph;
class EventBus;
struct RenderContext;

class BaseRenderPass {
public:
	virtual ~BaseRenderPass() = default;

	virtual void execute(const RenderContext& ctx, const RenderGraph& graph) = 0;

	std::vector<std::string>& inputs() { return mInputs; }
	std::vector<std::string>& outputs() { return mOutputs; }

protected:
	std::vector<std::string> mInputs;
	std::vector<std::string> mOutputs;
};
