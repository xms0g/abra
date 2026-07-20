#include "renderGraph.h"
#include <cassert>
#include <numeric>
#include "buffers/frameBuffer.h"
#include "renderPasses/baseRenderPass.hpp"

FrameBuffer& RenderGraph::getResource(const std::string& key) const {
	return *mResources.at(key);
}

void RenderGraph::addPass(std::unique_ptr<BaseRenderPass> pass) {
	mRenderPasses.push_back(std::move(pass));
}

void RenderGraph::addResources(const std::string& key, std::unique_ptr<FrameBuffer> resource) {
	mResources.emplace(key, std::move(resource));
}

void RenderGraph::compile() {
	executionOrder.resize(mRenderPasses.size());

	for (size_t i = 0; i < mRenderPasses.size(); ++i) {
		for (auto& resource: mRenderPasses[i]->reads())
			assert(mResources.contains(resource));

		for (auto& resource: mRenderPasses[i]->writes())
			assert(mResources.contains(resource));

		executionOrder.push_back(i);
	}
}

void RenderGraph::execute(const RenderContext& ctx) const {
	for (const size_t index : executionOrder)
		mRenderPasses[index]->execute(ctx, *this);
}
