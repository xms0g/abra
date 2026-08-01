#pragma once
#include <cstdint>
#include "event.hpp"

struct GuiDebugEvent : Event {
	explicit GuiDebugEvent(const size_t id, const uint32_t m)
		: entityID(id),
		  mode(m) {
	}

	size_t entityID;
	uint32_t mode;
};
