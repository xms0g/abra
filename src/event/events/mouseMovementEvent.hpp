#pragma once
#include "../event.hpp"

struct MouseMovementEvent : Event {
	explicit MouseMovementEvent(const float x_, const float y_)
		: x(x_),
		  y(y_) {
	}

	float x, y;
};
