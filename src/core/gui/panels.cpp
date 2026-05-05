#include "panels.h"
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "ui.h"
#include "entityState.hpp"
#include "../../ECS/registry.h"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiPostProcessEvent.hpp"

struct Effect {
	const char* name;
	bool enabled;
	float exposure{1.1f};
	float intensity{0.0f};
};

static std::array<Effect, 10> effects = {
	{
		{"Bloom", false},
		{"Tone Mapping", false},
		{"Grayscale", false},
		{"Sepia", false},
		{"Blur", false},
		{"Edge Detection", false},
		{"Sharpen", false},
		{"Chromatic Aberration", false},
		{"Gamma Correction", true},
		{"FXAA", false}
	}
};

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

void GuiPanels::renderDebugViewsPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates) {
	if (!entity.hasComponent<DebugComponent>()) return;
	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Debug Views", ImGuiTreeNodeFlags_DefaultOpen)) {
		static constexpr const char* modes[] = {"None", "Normals", "Wireframe"};
		auto& [id, debugMode] = entityStates[entity.id()];
		int currentMode = debugMode;

		ImGui::Text("Mode"); ImGui::SameLine(70);
		if (ImGui::Combo("##mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
			debugMode = currentMode;
			eventBus.emitEvent<GuiDebugEvent>(entity.id(), static_cast<DebugMode>(currentMode));
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderLightPanel(const Entity& entity) {
	ImGui::PushID(static_cast<int>(entity.id()));
	if (entity.hasComponent<DirectionalLightComponent>()) {
		renderDirLight(entity);
	} else if (entity.hasComponent<SpotLightComponent>()) {
		renderSpotLight(entity);
	} else if (entity.hasComponent<PointLightComponent>()) {
		renderPointLight(entity);
	}

	ImGui::PopID();
}

void GuiPanels::renderDirLight(const Entity& entity) {
	auto& dir = entity.getComponent<DirectionalLightComponent>();

	Ui::dragFloat4("Direction", dir.direction, 0.01f, 100);
	Ui::colorField4("Ambient", dir.ambient, 0.01f, 100);
	Ui::colorField4("Diffuse", dir.diffuse, 0.01f, 100);
	Ui::colorField4("Specular", dir.specular, 0.01f, 100);
	Ui::sliderFloat("Intensity", &dir.intensity, 100.0, 1.0, 30.0);
}

void GuiPanels::renderSpotLight(const Entity& entity) {
	auto& splc = entity.getComponent<SpotLightComponent>();

	Ui::dragFloat4("Direction", splc.direction, 0.01f, 100);
	Ui::colorField4("Ambient", splc.ambient, 0.01f, 100);
	Ui::colorField4("Diffuse", splc.diffuse, 0.01f, 100);
	Ui::colorField4("Specular", splc.specular, 0.01f, 100);
	Ui::dragFloat3("Attenua", splc.attenuation, 0.01f, 100);
	Ui::dragFloat3("Cutoff", splc.cutOff, 0.01f, 100);
	Ui::sliderFloat("Intensity", &splc.intensity, 100.0, 1.0, 30.0);
	ImGui::Checkbox("Cast Shadow", &splc.castShadow);
}

void GuiPanels::renderPointLight(const Entity& entity) {
	auto& plc = entity.getComponent<PointLightComponent>();

	Ui::colorField4("Ambient", plc.ambient, 0.01f, 100);
	Ui::colorField4("Diffuse", plc.diffuse, 0.01f, 100);
	Ui::colorField4("Specular", plc.specular, 0.01f, 100);
	Ui::dragFloat3("Attenua", plc.attenuation, 0.01f, 100);
	Ui::sliderFloat("Intensity", &plc.intensity, 100.0, 1.0, 30.0);
	ImGui::Checkbox("Cast Shadow", &plc.castShadow);
}

void GuiPanels::renderPostProcessPanel(EventBus& eventBus) {
	if (ImGui::Begin("Post-Processing")) {
		for (uint32_t i = 0; i < effects.size(); ++i) {
			bool changed{false};

			auto& [name, enabled, exposure, intensity] = effects[i];
			if (ImGui::Checkbox(name, &enabled)) {
				changed = true;
			}

			if (i == 1) {
				if (Ui::sliderFloat("Exposure", &exposure, 100.0f)) {
					changed = true;
				}
			} else if (i == 7) {
				if (Ui::sliderFloat("Intensity", &intensity, 100.0f, 0.01f, 0.1f)) {
					changed = true;
				}
			}

			if (changed) {
				eventBus.emitEvent<GuiPostProcessEvent>(name, enabled, exposure, intensity);
			}
		}
	}
	ImGui::End();
}
