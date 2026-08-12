#include "frameGraph.h"
#include <numeric>
#include <queue>
#include <utility>
#include "graphicsEncoder.h"
#include "buffers/frameBuffer.h"
#include "passes/IPass.hpp"
#include "../event/eventBus.hpp"

FrameBuffer& FrameGraph::getResource(const uint32_t id) const {
	return *mResources[id];
}

uint32_t FrameGraph::getResourceID(const std::string_view name) const {
	const auto it = mResourcesIDs.find(name);
	assert(it != mResourcesIDs.end());

	return it->second;
}

void FrameGraph::addPass(std::string name,
                         std::unique_ptr<IPass> pass,
                         std::vector<std::string> inputs,
                         std::vector<std::string> outputs,
                         const bool active) {
	mPasses.emplace_back(
		std::move(name),
		std::move(pass),
		std::move(inputs),
		std::move(outputs),
		active);
}

FrameBuffer& FrameGraph::addResource(std::string key, std::unique_ptr<FrameBuffer> resource) {
	mResources.emplace_back(std::move(resource));
	mResourcesIDs.emplace(std::move(key), mResources.size() - 1);

	return *mResources.back();
}

void FrameGraph::compile() {
	std::erase_if(mPasses, [](auto& p) { return !p.isActive; });

	std::unordered_map<std::string, size_t> producer;
	std::unordered_map<std::string, std::string> latestVersion;
	std::vector<std::vector<size_t> > edges(mPasses.size());
	std::vector<size_t> indegree(mPasses.size(), 0);

	mExecutionOrder.resize(mPasses.size());

	for (size_t i = 0; i < mPasses.size(); ++i) {
		for (auto& input: mPasses[i].inputs) {
			if (latestVersion.contains(input)) {
				input = latestVersion[input];
			}
		}

		for (auto& output: mPasses[i].outputs) {
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

	for (size_t i = 0; i < mPasses.size(); ++i) {
		for (auto& input: mPasses[i].inputs) {
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

void FrameGraph::configure(const RenderContext& ctx, GraphicsEncoder& encoder, EventBus& eventBus) const {
	for (const size_t index: mExecutionOrder) {
		mPasses[index].pass->configure(ctx, *this, encoder, eventBus);
	}
}

void FrameGraph::execute(const RenderContext& ctx, GraphicsEncoder& encoder) const {
	for (const size_t index: mExecutionOrder) {
		mPasses[index].pass->execute(ctx, *this, encoder);
	}
}
