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

void Ui::colorField4(const char* label, glm::vec4& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	ImGui::DragFloat3(("##" + std::string(label) + "v").c_str(), glm::value_ptr(value), speed);
	ImGui::SameLine();
	ImGui::ColorEdit3(("##" + std::string(label) + "c").c_str(), glm::value_ptr(value),
					  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
}

void Ui::dragFloat3(const char* label, glm::vec3& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	ImGui::DragFloat3(("##" + std::string(label) + "v").c_str(), glm::value_ptr(value), speed);
}

void Ui::dragFloat4(const char* label, glm::vec4& value, const float speed, const float sameLineOffset) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	ImGui::DragFloat4(("##" + std::string(label) + "v").c_str(), glm::value_ptr(value), speed);
}

void Ui::sliderFloat(const char* label, float* value, const float sameLineOffset, const float min, const float max) {
	ImGui::Text("%s", label);
	ImGui::SameLine(sameLineOffset);
	ImGui::SliderFloat(("##" + std::string(label) + "v").c_str(), value, min, max);
}
