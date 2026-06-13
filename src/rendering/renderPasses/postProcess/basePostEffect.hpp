#pragma once
#include <memory>
#include <array>
#include <string>
#include "../../types.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

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

	virtual uint32_t render(
		uint32_t vao,
		uint32_t sceneTexture,
		bool& toggle,
		PingPongBuffer& renderTargets) const = 0;

	void updateFromEvent(const GuiPostProcessEvent& event) {
		this->enabled(event.enabled);
		updateFromEventImpl(event);
	}

protected:
	virtual void updateFromEventImpl(const GuiPostProcessEvent& event) = 0;

private:
	std::string mName;
	bool mEnabled{false};
};
