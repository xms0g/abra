#pragma once
#include "../event.hpp"

struct GuiLightEvent : Event {
	explicit GuiLightEvent(
		const size_t id,
		const uint32_t idx,
		const glm::vec4& dir,
		const glm::vec4& pos,
		const glm::vec4& amb,
		const glm::vec4& dif,
		const glm::vec4& spe,
		const glm::vec3& att,
		const glm::vec3& cut,
		const bool cast,
		const float intens)
		: entityID(id),
		  matIdx(idx),
		  direction(dir),
		  position(pos),
		  ambient(amb),
		  diffuse(dif),
		  specular(spe),
		  attenuation(att),
		  cutOff(cut),
		  castShadow(cast),
		  intensity(intens) {
	}

	size_t entityID;
	uint32_t matIdx;
	glm::vec4 direction{};
	glm::vec4 position{};
	glm::vec4 ambient{};
	glm::vec4 diffuse{};
	glm::vec4 specular{};
	glm::vec3 attenuation; // (Kc, Kl, kq)
	glm::vec3 cutOff; // (cutOff, outerCutOff, padding)
	bool castShadow{};
	float intensity{};
};
