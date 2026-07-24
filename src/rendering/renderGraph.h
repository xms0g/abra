#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

class EventBus;
class FrameBuffer;
struct RenderContext;
class IRenderPass;

class RenderGraph {
public:
	[[nodiscard]]
	FrameBuffer& getResource(const std::string& key) const;

	void addPass(
		const std::string& name,
		bool active,
		std::unique_ptr<IRenderPass> pass,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs);

	void addResource(const std::string& key, std::unique_ptr<FrameBuffer> resource);

	void compile();

	void configure(const RenderContext& ctx, EventBus& eventBus) const;

	void execute(const RenderContext& ctx) const;

private:
	struct PassNode {
		std::string name;
		bool isActive{true};
		std::unique_ptr<IRenderPass> pass;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
	};

	std::unordered_map<std::string, std::unique_ptr<FrameBuffer> > mResources;
	std::vector<PassNode> mRenderPasses;
	std::vector<size_t> mExecutionOrder;
};
