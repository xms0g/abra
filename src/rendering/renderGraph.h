#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

class FrameBuffer;
struct RenderContext;
class BaseRenderPass;

class RenderGraph {
public:

	[[nodiscard]]
	FrameBuffer& getResource(const std::string& key) const;

	void addPass(std::unique_ptr<BaseRenderPass> pass);

	void addResources(const std::string& key, std::unique_ptr<FrameBuffer> resource);

	void compile();

	void execute(const RenderContext& ctx) const;

private:
	std::unordered_map<std::string, std::unique_ptr<FrameBuffer>> mResources;
	std::vector<std::unique_ptr<BaseRenderPass> > mRenderPasses;
	std::vector<size_t> executionOrder;
};
