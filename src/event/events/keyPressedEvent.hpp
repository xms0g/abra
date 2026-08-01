#pragma once
#include <cstdint>
#include "event.hpp"

struct KeyPressedEvent : Event {
	explicit KeyPressedEvent(const uint32_t dir, const float dt)
		: direction(dir),
		  deltaTime(dt) {
	}

	uint32_t direction;
	float deltaTime;
};
