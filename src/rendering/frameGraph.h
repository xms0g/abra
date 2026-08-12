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
	FrameBuffer& getResource(uint32_t id) const;

	[[nodiscard]]
	uint32_t getResourceID(std::string_view name) const;

	void addPass(std::string name,
	             std::unique_ptr<IPass> pass,
	             std::vector<std::string> inputs,
	             std::vector<std::string> outputs,
	             bool active);

	FrameBuffer& addResource(std::string key, std::unique_ptr<FrameBuffer> resource);

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

	struct StringHash {
		using is_transparent = void;

		size_t operator()(const std::string_view value) const noexcept {
			return std::hash<std::string_view>{}(value);
		}
	};

	struct StringEqual {
		using is_transparent = void;

		bool operator()(const std::string_view lhs, const std::string_view rhs) const noexcept {
			return lhs == rhs;
		}
	};

	std::unordered_map<std::string, uint32_t, StringHash, StringEqual> mResourcesIDs;
	std::vector<std::unique_ptr<FrameBuffer>> mResources;
	std::vector<PassNode> mPasses;
	std::vector<size_t> mExecutionOrder;
};
