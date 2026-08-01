#pragma once
#include <cstdint>
#include "event.hpp"

struct GuiLightEvent : Event {
	size_t entityID;
	uint32_t matIdx;
	uint32_t lightIdx;

	explicit GuiLightEvent(
		const size_t id,
		const uint32_t midx,
		const uint32_t lidx)
		: entityID(id),
		  matIdx(midx),
		  lightIdx(lidx) {
	}
};
