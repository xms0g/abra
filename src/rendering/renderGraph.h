#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

class EventBus;
class FrameBuffer;
struct RenderContext;
class BaseRenderPass;

struct PassNode {
	std::string name;
	bool isActive{true};
	std::unique_ptr<BaseRenderPass> pass;
};

class RenderGraph {
public:
	[[nodiscard]]
	FrameBuffer& getResource(const std::string& key) const;

	void addPass(PassNode&& pass);

	void addResources(const std::string& key, std::unique_ptr<FrameBuffer> resource);

	void compile();

	void configure(const RenderContext& ctx, EventBus& eventBus) const;

	void execute(const RenderContext& ctx) const;

private:
	std::unordered_map<std::string, std::unique_ptr<FrameBuffer>> mResources;
	std::vector<PassNode> mRenderPasses;
	std::vector<size_t> mExecutionOrder;
};
