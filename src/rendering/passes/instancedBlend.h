#pragma once
#include <memory>
#include <vector>
#include <span>
#include "glm/glm.hpp"
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"
#include "../context/renderQueue.hpp"

class VertexBuffer;
struct RenderInstanceGroup;

class InstancedBlendPass final : public IPass {
public:
	InstancedBlendPass();

	~InstancedBlendPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	void prepareInstanceBuffer(const std::vector<uint32_t>& vaos);

	void uploadInstanceData() const;

	GraphicsPipeline mPipeline{};
	GraphicsEncoder mEncoder{};
	std::unique_ptr<VertexBuffer> mVBO;
	std::span<RenderInstanceGroup> mObjects;
};
