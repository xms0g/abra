#pragma once
#include <string>
#include "../../graphicsPipeline.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

namespace Model {
class Quad;
}

class FrameGraph;
class FrameBuffer;
class DescriptorSet;
class GraphicsEncoder;

class BasePostEffect {
public:
	BasePostEffect() = default;

	BasePostEffect(std::string n, const bool e)
		: mName(std::move(n)), mEnabled(e) {
	}

	virtual ~BasePostEffect() = default;

	[[nodiscard]]
	const std::string& name() const { return mName; }

	[[nodiscard]]
	bool enabled() const { return mEnabled; }

	void enabled(const bool e) { mEnabled = e; }

	virtual void configure(const FrameGraph& graph) = 0;

	virtual DescriptorSet* render(GraphicsEncoder& encoder,
	                              DescriptorSet& dscSet,
	                              DescriptorSet& renderTargetDscSet,
	                              FrameBuffer* renderTarget) = 0;

	void updateFromEvent(const GuiPostProcessEvent& event) {
		this->enabled(event.enabled);
		updateFromEventImpl(event);
	}

protected:
	virtual void updateFromEventImpl(const GuiPostProcessEvent& event) = 0;

	GraphicsPipeline mPipeline;

private:
	std::string mName;
	bool mEnabled{false};
};
