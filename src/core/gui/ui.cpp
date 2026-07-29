#include "ui.h"
#include <string>
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"

bool ui::beginEntity(const std::string& label) {
	return ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
}

void ui::endEntity() {
	ImGui::Separator();
}

void ui::pushID(const size_t id) {
	ImGui::PushID(static_cast<int>(id));
}

void ui::popID() {
	ImGui::PopID();
}

bool ui::colorField3(const std::string& label, glm::vec3& value, const float speed, const float sameLineOffset) {
	bool isDirty{false};
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine(sameLineOffset);
	isDirty |= ImGui::DragFloat3(("##" + label + "v").c_str(), glm::value_ptr(value), speed);
	ImGui::SameLine();
	isDirty |= ImGui::ColorEdit3(("##" + label + "c").c_str(), glm::value_ptr(value),
					  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	return isDirty;
}

bool ui::dragFloat(const std::string& label, float* value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine(sameLineOffset);
	return ImGui::DragFloat(("##" + label + "v").c_str(), value, speed);
}

bool ui::dragFloat3(const std::string& label, glm::vec3& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine(sameLineOffset);
	return ImGui::DragFloat3(("##" + label + "v").c_str(), glm::value_ptr(value), speed);
}

bool ui::dragFloat4(const std::string& label, glm::vec4& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine(sameLineOffset);
	return ImGui::DragFloat4(("##" + label + "v").c_str(), glm::value_ptr(value), speed);
}

bool ui::sliderFloat(const std::string& label, float* value, const float sameLineOffset, const float min, const float max) {
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine(sameLineOffset);

	return ImGui::SliderFloat(("##" + label + "v").c_str(), value, min, max);
}