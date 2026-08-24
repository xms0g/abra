#pragma once
#include "event.hpp"

enum class Key : uint8_t {
	W, A, S, D
};

struct KeyPressedEvent : Event {
	explicit KeyPressedEvent(const Key k, const float dt)
		: key(k),
		  deltaTime(dt) {
	}

	Key key;
	float deltaTime;
};
