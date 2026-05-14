#pragma once
#include "../event.hpp"

struct GuiLightEvent : Event {
	size_t entityID;
	uint32_t matIdx;
	glm::vec3 direction{};
	glm::vec3 position{};
	glm::vec3 ambient{};
	glm::vec3 diffuse{};
	glm::vec3 specular{};
	float constant;
	float linear;
	float quadratic;
	float cutOff;
	float outerCutOff;
	float intensity;
	bool castShadow;

	explicit GuiLightEvent(
		const size_t id,
		const uint32_t idx,
		const glm::vec3& dir,
		const glm::vec3& pos,
		const glm::vec3& amb,
		const glm::vec3& dif,
		const glm::vec3& spe,
		const float kc,
		const float kl,
		const float kq,
		const float co,
		const float oc,
		const float i,
		const bool cs)
		: entityID(id),
		  matIdx(idx),
		  direction(dir),
		  position(pos),
		  ambient(amb),
		  diffuse(dif),
		  specular(spe),
		  constant(kc),
		  linear(kl),
		  quadratic(kq),
		  cutOff(co),
		  outerCutOff(oc),
		  intensity(i),
		  castShadow(cs) {
	}
};
