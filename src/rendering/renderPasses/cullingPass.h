#pragma once
#include <vector>
#include "IRenderPass.hpp"

namespace math {
struct Frustum;
}

struct RenderGroup;
struct RenderableObject;

class CullingPass final : public IRenderPass {
public:
	~CullingPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	static void cullScene(
		const RenderContext& ctx,
		const math::Frustum& frustum,
		const std::vector<RenderGroup>& groups,
		std::vector<RenderableObject>& outQueue);
};