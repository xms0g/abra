#include "renderGraph.h"
#include <numeric>
#include <queue>
#include "buffers/frameBuffer.h"
#include "renderPasses/IRenderPass.hpp"
#include "../event/eventBus.hpp"

FrameBuffer& RenderGraph::getResource(const std::string& key) const {
	return *mResources.at(key);
}

void RenderGraph::addPass(
	const std::string& name,
	const bool active,
	std::unique_ptr<IRenderPass> pass,
	const std::vector<std::string>& inputs,
	const std::vector<std::string>& outputs) {
	mRenderPasses.push_back({
		.name = name,
		.isActive = active,
		.pass = std::move(pass),
		.inputs = inputs,
		.outputs = outputs
	});
}

void RenderGraph::addResources(const std::string& key, std::unique_ptr<FrameBuffer> resource) {
	mResources.emplace(key, std::move(resource));
}

void RenderGraph::compile() {
	std::erase_if(mRenderPasses, [](auto& p) { return !p.isActive; });

	std::unordered_map<std::string, size_t> producer;
	std::unordered_map<std::string, std::string> latestVersion;
	std::vector<std::vector<size_t> > edges(mRenderPasses.size());
	std::vector<size_t> indegree(mRenderPasses.size(), 0);

	mExecutionOrder.resize(mRenderPasses.size());

	for (size_t i = 0; i < mRenderPasses.size(); ++i) {
		for (auto& input: mRenderPasses[i].inputs) {
			if (latestVersion.contains(input)) {
				input = latestVersion[input];
			}
		}

		for (auto& output: mRenderPasses[i].outputs) {
			if (producer.contains(output)) {
				std::string versioned;
				versioned = output + "#v" + std::to_string(i);
				latestVersion[output] = versioned;
				producer[versioned] = i;
				continue;
			}

			producer[output] = i;
		}
	}

	for (size_t i = 0; i < mRenderPasses.size(); ++i) {
		for (auto& input: mRenderPasses[i].inputs) {
			if (!producer.contains(input))
				continue;

			const size_t dependency = producer[input];

			if (dependency == i)
				continue;

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

void RenderGraph::configure(const RenderContext& ctx, EventBus& eventBus) const {
	for (const size_t index: mExecutionOrder) {
		mRenderPasses[index].pass->configure(ctx, *this, eventBus);
	}
}

void RenderGraph::execute(const RenderContext& ctx) const {
	for (const size_t index: mExecutionOrder) {
		mRenderPasses[index].pass->execute(ctx, *this);
	}
}
