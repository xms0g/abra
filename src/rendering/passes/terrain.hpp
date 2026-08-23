#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.hpp"
#include "../context/renderQueue.hpp"
#include "../pushConstants/materialPushConstants.hpp"
#include "../pushConstants/transformPushConstants.hpp"

struct DrawCommand;
class Shader;
struct RenderGroup;

class TerrainPass final : public IPass {
public:
	TerrainPass();

	~TerrainPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct ResourceIndexes {
		uint32_t sceneBuffer;
	};

	struct PushConstants {
		MaterialPushConstants material;
		TransformPushConstants transform;
	};

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
