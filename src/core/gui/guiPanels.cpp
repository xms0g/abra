#include "guiPanels.h"
#include "guiBackend.h"
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "../../ECS/registry.h"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../rendering/renderPasses/postProcess/ca.h"
#include "../../rendering/renderPasses/postProcess/toneMapping.h"

void GuiPanels::renderGraphicsInfoPanel(const uint32_t fps) {
	if (ImGui::Begin("Graphics")) {
		ImGui::Text("%s FPS", std::to_string(fps).c_str());
		ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
		ImGui::Text("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		ImGui::Text("OpenGL Driver Vendor: %s", glGetString(GL_VENDOR));
		ImGui::Text("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	}
	ImGui::End();
}

void GuiPanels::renderTransformPanel(const Entity& entity) {
	if (!entity.hasComponent<TransformComponent>()) return;

	auto& tc = entity.getComponent<TransformComponent>();

	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		Ui::dragFloat3("Position", tc.position);
		Ui::dragFloat3("Rotation", tc.rotation, 1.0f);
		Ui::dragFloat3("Scale", tc.scale);
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderDebugViewsPanel(const Entity& entity) {
	if (!entity.hasComponent<DebugComponent>()) return;

	auto& db = entity.getComponent<DebugComponent>();

	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Debug Views", ImGuiTreeNodeFlags_DefaultOpen)) {
		const char* modes[] = {"None", "Normals", "Wireframe"};
		int currentMode = db.mode;
		ImGui::Text("Mode");
		ImGui::SameLine(70);
		if (ImGui::Combo("##mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
			db.mode = static_cast<DebugMode>(currentMode);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderLightPanel(const Entity& entity) {
	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (entity.hasComponent<DirectionalLightComponent>()) {
			renderDirLight(entity);
		}else if (entity.hasComponent<SpotLightComponent>()) {
			renderSpotLight(entity);
		} else if (entity.hasComponent<PointLightComponent>()) {
			renderPointLight(entity);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderDirLight(const Entity& entity) {
	auto& dir = entity.getComponent<DirectionalLightComponent>();

	Ui::dragFloat4("Direction", dir.direction, 0.01f, 100);
	Ui::colorField4("Ambient", dir.ambient, 0.01f, 100);
	Ui::colorField4("Diffuse", dir.diffuse, 0.01f, 100);
	Ui::colorField4("Specular", dir.specular, 0.01f, 100);
}

void GuiPanels::renderSpotLight(const Entity& entity) {
	auto& splc = entity.getComponent<SpotLightComponent>();

	Ui::dragFloat4("Direction", splc.direction, 0.01f, 100);
	Ui::colorField4("Ambient", splc.ambient, 0.01f, 100);
	Ui::colorField4("Diffuse", splc.diffuse, 0.01f, 100);
	Ui::colorField4("Specular", splc.specular, 0.01f, 100);
	Ui::dragFloat3("Attenua", splc.attenuation, 0.01f, 100);
	Ui::dragFloat4("Cutoff", splc.cutOff, 0.01f, 100);
	ImGui::Checkbox("Cast Shadow", &splc.castShadow);
}

void GuiPanels::renderPointLight(const Entity& entity) {
	auto& plc = entity.getComponent<PointLightComponent>();

	Ui::colorField4("Ambient", plc.ambient, 0.01f, 100);
	Ui::colorField4("Diffuse", plc.diffuse, 0.01f, 100);
	Ui::colorField4("Specular", plc.specular, 0.01f, 100);
	Ui::dragFloat3("Attenua", plc.attenuation, 0.01f, 100);
	ImGui::Checkbox("Cast Shadow", &plc.castShadow);
}

void GuiPanels::renderPostProcessPanel(const std::vector<std::shared_ptr<IPostEffect>>& effects) {
	if (ImGui::Begin("Post-Processing")) {
		for (auto& effect: effects) {
			ImGui::Checkbox(effect->name().c_str(), &effect->enabled());

			if (effect->name() == "Tone Mapping") {
				Ui::sliderFloat("Exposure", &std::dynamic_pointer_cast<ToneMapping>(effect)->exposure, 100.0f);
			} else if (effect->name() == "Chromatic Aberration") {
				Ui::sliderFloat("Intensity", &std::dynamic_pointer_cast<CA>(effect)->intensity(), 100.0f, 0.01f, 0.1f);
			}
		}
	}
	ImGui::End();
}
