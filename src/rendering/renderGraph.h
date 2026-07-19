#pragma once
#include <any>
#include <memory>
#include <unordered_map>
#include <vector>

class FrameBuffer;
struct RenderContext;
class IRenderPass;

class RenderGraph {
public:

	FrameBuffer& getResource(const std::string& key) const;

	void addPass(std::unique_ptr<IRenderPass> pass);

	void addResources(const std::string& key, std::unique_ptr<FrameBuffer> resource);

	void compile();

	void execute(const RenderContext& ctx) const;

private:
	std::unordered_map<std::string, std::unique_ptr<FrameBuffer>> mResources;
	std::vector<std::unique_ptr<IRenderPass> > mRenderPasses;
	std::vector<size_t> executionOrder;
};
