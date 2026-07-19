#include "renderGraph.h"
#include <numeric>
#include "buffers/frameBuffer.h"
#include "renderPasses/IRenderPass.hpp"

FrameBuffer& RenderGraph::getResource(const std::string& key) const {
	return *mResources.at(key);
}

void RenderGraph::addPass(std::unique_ptr<IRenderPass> pass) {
	mRenderPasses.push_back(std::move(pass));
}

void RenderGraph::addResources(const std::string& key, std::unique_ptr<FrameBuffer> resource) {
	mResources.emplace(key, std::move(resource));
}

void RenderGraph::compile() {
	executionOrder.resize(mRenderPasses.size());

}

void RenderGraph::execute(const RenderContext& ctx) const {
	for (const auto& pass: mRenderPasses) {
		//pass->execute(ctx);
	}
}
