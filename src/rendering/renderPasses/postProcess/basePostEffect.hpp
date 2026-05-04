#pragma once
#include <memory>
#include <string>
#include "../../../event/events/guiPostProcessEvent.hpp"

class FrameBuffer;
using PingPongBuffer = std::array<std::unique_ptr<FrameBuffer>, 2>;

class BasePostEffect {
public:
	BasePostEffect() = default;

	BasePostEffect(std::string n, const bool e) : mName(std::move(n)), mEnabled(e) {
	}

	virtual ~BasePostEffect() = default;

	[[nodiscard]]
	const std::string& name() const { return mName; }

	[[nodiscard]]
	bool enabled() const { return mEnabled; }

	void enabled(const bool e) { mEnabled = e; }

	virtual uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		PingPongBuffer& pingPong) const = 0;

	virtual void updateFromEvent(const GuiPostProcessEvent& event) {
		this->enabled(event.enabled);
	}

private:
	std::string mName;
	bool mEnabled{false};
};
