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
	FrameBuffer& getResource(const std::string& key) const;

	void addPass(
		const std::string& name,
		bool active,
		std::unique_ptr<IPass> pass,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs);

	void addResource(const std::string& key, std::unique_ptr<FrameBuffer> resource);

	void compile();

	void configure(const RenderContext& ctx, GraphicsEncoder& encoder, EventBus& eventBus) const;

	void execute(const RenderContext& ctx, GraphicsEncoder& encoder) const;

private:
	struct PassNode {
		std::string name;
		bool isActive{true};
		std::unique_ptr<IPass> pass;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
	};

	std::unordered_map<std::string, std::unique_ptr<FrameBuffer> > mResources;
	std::vector<PassNode> mPasses;
	std::vector<size_t> mExecutionOrder;
};
