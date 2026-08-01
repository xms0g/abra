#pragma once
#include "glm/glm.hpp"
#include "event.hpp"

struct GuiTransformEvent : Event {
	explicit GuiTransformEvent(const size_t id, const glm::vec3& p, const glm::vec3& r, const glm::vec3& s)
		: entityID(id),
		  position(p),
		  rotation(r),
		  scale(s) {
	}

	size_t entityID;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};
