#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

class GraphicsEncoder;
class EventBus;
class FrameBuffer;
struct RenderContext;
class IPass;

class FrameGraph {
public:
	[[nodiscard]]
	FrameBuffer& getResource(std::string_view key) const;

	void addPass(std::string name,
	             std::unique_ptr<IPass> pass,
	             std::vector<std::string> inputs,
	             std::vector<std::string> outputs,
	             bool active);

	void addResource(std::string key, std::unique_ptr<FrameBuffer> resource);

	void compile();

	void configure(const RenderContext& ctx, GraphicsEncoder& encoder, EventBus& eventBus) const;

	void execute(const RenderContext& ctx, GraphicsEncoder& encoder) const;

private:
	struct PassNode {
		std::string name;
		std::unique_ptr<IPass> pass;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
		bool isActive{true};
	};

	std::unordered_map<std::string, std::unique_ptr<FrameBuffer> > mResources;
	std::vector<PassNode> mPasses;
	std::vector<size_t> mExecutionOrder;
};
