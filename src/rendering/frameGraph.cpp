#include "frameGraph.h"
#include <numeric>
#include <queue>
#include <utility>
#include "graphicsEncoder.h"
#include "buffers/frameBuffer.h"
#include "passes/IPass.hpp"
#include "../event/eventBus.hpp"

FrameBuffer& FrameGraph::getResource(const std::string& key) const {
	return *mResources.at(key);
}

void FrameGraph::addPass(
	std::string name,
	const bool active,
	std::unique_ptr<IPass> pass,
	std::vector<std::string> inputs,
	std::vector<std::string> outputs) {
	mPasses.emplace_back(
		std::move(name),
		active,
		std::move(pass),
		std::move(inputs),
		std::move(outputs));
}

void FrameGraph::addResource(std::string key, std::unique_ptr<FrameBuffer> resource) {
	mResources.emplace(std::move(key), std::move(resource));
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
