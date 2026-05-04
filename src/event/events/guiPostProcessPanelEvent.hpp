#pragma once
#include "../event.hpp"

struct GuiPostProcessPanelEvent : Event {
	explicit GuiPostProcessPanelEvent(const char* n, const bool e, const float exp, const float intens)
		: name(n),
		  enabled(e),
		  exposure(exp),
		  intensity(intens) {
	}

	const char* name;
	bool enabled;
	float exposure;
	float intensity;
};
