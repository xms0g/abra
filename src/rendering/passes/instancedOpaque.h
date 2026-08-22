#pragma once
#include <memory>
#include <vector>
#include <span>
#include "glm/glm.hpp"
#include "IPass.hpp"
#include "../graphicsPipeline.h"

class VertexBuffer;
struct RenderInstanceGroup;

class InstancedOpaquePass final : public IPass {
public:
	InstancedOpaquePass();

	~InstancedOpaquePass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	struct ResourceIndexes {
		uint32_t sceneBuffer;
	};

	void prepareInstanceBuffer(const std::vector<uint32_t>& vaos);

	void uploadInstanceData() const;

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	std::unique_ptr<VertexBuffer> mVBO;
	std::span<RenderInstanceGroup> mObjects;
};
