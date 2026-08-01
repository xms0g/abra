#pragma once
#include "event.hpp"

struct GuiPostProcessEvent : Event {
	explicit GuiPostProcessEvent(const uint32_t i, const bool e, const float exp, const float intens)
		: id(i),
		  enabled(e),
		  exposure(exp),
		  intensity(intens) {
	}

	uint32_t id;
	bool enabled;
	float exposure;
	float intensity;
};
