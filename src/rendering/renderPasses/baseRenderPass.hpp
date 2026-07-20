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

	std::vector<std::string>& reads() { return mReads; }
	std::vector<std::string>& writes() { return mWrites; }

protected:
	std::vector<std::string> mReads;
	std::vector<std::string> mWrites;
};
