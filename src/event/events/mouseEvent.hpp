#pragma once
#include "../event.hpp"

struct MouseEvent : Event {
	explicit MouseEvent(const float x_, const float y_)
		: x(x_),
		  y(y_) {
	}

	float x, y;
};
