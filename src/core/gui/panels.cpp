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
#include "../../event/events/guiTransformEvent.hpp"
#include "../../event/events/guiLightEvent.hpp"

struct Effect {
	const char* name;
	bool enabled;
	float exposure{1.1f};
	float intensity{0.01f};
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

void GuiPanels::renderTransformPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates) {
	if (!entity.hasComponent<TransformComponent>()) return;

	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& [id, debugMode, alreadyDirty, transform, light] = entityStates[entity.id()];

		bool isDirty{false};

		isDirty |= Ui::dragFloat3("Position", transform.position);
		isDirty |= Ui::dragFloat3("Rotation", transform.rotation, 1.0f);
		isDirty |= Ui::dragFloat3("Scale", transform.scale);

		if (isDirty) {
			alreadyDirty = true;
			light.position = glm::vec4(transform.position, 1.0f);
			eventBus.emitEvent<GuiTransformEvent>(entity.id(), transform.position, transform.rotation, transform.scale);
		} else alreadyDirty = false;

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderDebugViewsPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates) {
	if (!entity.hasComponent<DebugComponent>()) return;

	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Debug Views", ImGuiTreeNodeFlags_DefaultOpen)) {
		static constexpr const char* modes[] = {"None", "Normals", "Wireframe"};
		auto& [id, debugMode, alreadyDirty, transform, light] = entityStates[entity.id()];
		auto currentMode = static_cast<int32_t>(debugMode);

		ImGui::Text("Mode");
		ImGui::SameLine(70);
		if (ImGui::Combo("##mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
			debugMode = currentMode;
			eventBus.emitEvent<GuiDebugEvent>(entity.id(), static_cast<DebugMode>(currentMode));
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderLightPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates) {
	ImGui::PushID(static_cast<int>(entity.id()));
	auto& eState = entityStates[entity.id()];

	if (entity.hasComponent<DirectionalLightComponent>()) {
		renderDirLight(entity, eventBus, eState);
	} else if (entity.hasComponent<SpotLightComponent>()) {
		renderSpotLight(entity, eventBus, eState);
	} else if (entity.hasComponent<PointLightComponent>()) {
		renderPointLight(entity, eventBus, eState);
	}

	ImGui::PopID();
}

void GuiPanels::renderDirLight(const Entity& entity, EventBus& eventBus, EntityState& entityState) {
	bool isDirty{false};

	isDirty |= Ui::dragFloat4("Direction", entityState.light.direction, 0.01f, 100);
	isDirty |= Ui::colorField4("Ambient", entityState.light.ambient, 0.01f, 100);
	isDirty |= Ui::colorField4("Diffuse", entityState.light.diffuse, 0.01f, 100);
	isDirty |= Ui::colorField4("Specular", entityState.light.specular, 0.01f, 100);
	isDirty |= Ui::sliderFloat("Intensity", &entityState.light.intensity, 100.0, 1.0, 30.0);

	if (isDirty) {
		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			entityState.light.direction,
			entityState.light.position,
			entityState.light.ambient,
			entityState.light.diffuse,
			entityState.light.specular,
			entityState.light.attenuation,
			entityState.light.cutOff,
			entityState.light.castShadow,
			entityState.light.intensity);
	}
}

void GuiPanels::renderSpotLight(const Entity& entity, EventBus& eventBus, EntityState& entityState) {
	bool isDirty = entityState.isDirty;

	isDirty |= Ui::dragFloat4("Direction", entityState.light.direction, 0.01f, 100);
	isDirty |= Ui::colorField4("Ambient", entityState.light.ambient, 0.01f, 100);
	isDirty |= Ui::colorField4("Diffuse", entityState.light.diffuse, 0.01f, 100);
	isDirty |= Ui::colorField4("Specular", entityState.light.specular, 0.01f, 100);
	isDirty |= Ui::dragFloat3("Attenua", entityState.light.attenuation, 0.01f, 100);
	isDirty |= Ui::dragFloat3("Cutoff", entityState.light.cutOff, 0.01f, 100);
	isDirty |= Ui::sliderFloat("Intensity", &entityState.light.intensity, 100.0, 1.0, 30.0);
	isDirty |= ImGui::Checkbox("Cast Shadow", &entityState.light.castShadow);

	if (isDirty) {
		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			entityState.light.direction,
			entityState.light.position,
			entityState.light.ambient,
			entityState.light.diffuse,
			entityState.light.specular,
			entityState.light.attenuation,
			entityState.light.cutOff,
			entityState.light.castShadow,
			entityState.light.intensity);
	}
}

void GuiPanels::renderPointLight(const Entity& entity, EventBus& eventBus, EntityState& entityState) {
	bool isDirty = entityState.isDirty;

	isDirty |= Ui::colorField4("Ambient", entityState.light.ambient, 0.01f, 100);
	isDirty |= Ui::colorField4("Diffuse", entityState.light.diffuse, 0.01f, 100);
	isDirty |= Ui::colorField4("Specular", entityState.light.specular, 0.01f, 100);
	isDirty |= Ui::dragFloat3("Attenua", entityState.light.attenuation, 0.01f, 100);
	isDirty |= Ui::sliderFloat("Intensity", &entityState.light.intensity, 100.0, 1.0, 30.0);
	isDirty |= ImGui::Checkbox("Cast Shadow", &entityState.light.castShadow);

	if (isDirty) {
		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			entityState.light.direction,
			entityState.light.position,
			entityState.light.ambient,
			entityState.light.diffuse,
			entityState.light.specular,
			entityState.light.attenuation,
			entityState.light.cutOff,
			entityState.light.castShadow,
			entityState.light.intensity);
	}
}

void GuiPanels::renderPostProcessPanel(EventBus& eventBus) {
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

	if (ImGui::Begin("Post-Processing")) {
		for (uint32_t i = 0; i < effects.size(); ++i) {
			auto& [name, enabled, exposure, intensity] = effects[i];
			bool isDirty{false};

			isDirty |= ImGui::Checkbox(name, &enabled);

			if (i == 1) {
				isDirty |= Ui::sliderFloat("Exposure", &exposure, 100.0f);
			} else if (i == 7) {
				isDirty |= Ui::sliderFloat("Intensity", &intensity, 100.0f, 0.01f, 0.1f);
			}

			if (isDirty) {
				eventBus.emitEvent<GuiPostProcessEvent>(i, enabled, exposure, intensity);
			}
		}
	}
	ImGui::End();
}
