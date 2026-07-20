#include "renderGraph.h"
#include <cassert>
#include <numeric>
#include <queue>
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
	std::unordered_map<std::string, size_t> producer;
	std::vector<std::vector<size_t> > edges(mRenderPasses.size());
	std::vector<size_t> indegree(mRenderPasses.size(), 0);

	mExecutionOrder.resize(mRenderPasses.size());

	for (size_t i = 0; i < mRenderPasses.size(); ++i) {
		for (auto& output: mRenderPasses[i]->outputs())
			producer[output] = i;
	}

	for (size_t i = 0; i < mRenderPasses.size(); ++i) {
		for (auto& input: mRenderPasses[i]->inputs()) {
			if (!producer.contains(input))
				continue;

			const size_t dependency = producer[input];

			edges[dependency].push_back(i);
			indegree[i]++;
		}
	}

	std::queue<size_t> queue;
	for (size_t i = 0; i < indegree.size(); ++i) {
		if (indegree[i] == 0)
			queue.push(i);
	}

	mExecutionOrder.clear();

	while (!queue.empty()) {
		auto node = queue.front();
		queue.pop();

		mExecutionOrder.push_back(node);

		for (auto next: edges[node]) {
			if (--indegree[next] == 0)
				queue.push(next);
		}
	}
}

void RenderGraph::execute(const RenderContext& ctx) const {
	for (const size_t index: mExecutionOrder) {
		mRenderPasses[index]->execute(ctx, *this);
	}
}
