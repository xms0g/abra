#include "ui.h"
#include <string>
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"

bool Ui::beginEntity(const char* label) {
	return ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
}

void Ui::endEntity() {
	ImGui::Separator();
}

bool Ui::colorField3(const char* label, glm::vec3& value, const float speed, const float sameLineOffset) {
	bool isDirty{false};
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	isDirty |= ImGui::DragFloat3(("##" + std::string(label) + "v").c_str(), glm::value_ptr(value), speed);
	ImGui::SameLine();
	isDirty |= ImGui::ColorEdit3(("##" + std::string(label) + "c").c_str(), glm::value_ptr(value),
					  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	return isDirty;
}

bool Ui::dragFloat(const char* label, float* value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	return ImGui::DragFloat(("##" + std::string(label) + "v").c_str(), value, speed);
}

bool Ui::dragFloat3(const char* label, glm::vec3& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	return ImGui::DragFloat3(("##" + std::string(label) + "v").c_str(), glm::value_ptr(value), speed);
}

bool Ui::dragFloat4(const char* label, glm::vec4& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	return ImGui::DragFloat4(("##" + std::string(label) + "v").c_str(), glm::value_ptr(value), speed);
}

bool Ui::sliderFloat(const char* label, float* value, const float sameLineOffset, const float min, const float max) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);

	return ImGui::SliderFloat(("##" + std::string(label) + "v").c_str(), value, min, max);
}