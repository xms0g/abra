#include "panels.h"
#include <span>
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "ui.h"
#include "entityState.hpp"
#include "../../ECS/registry.h"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/material.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiPostProcessEvent.hpp"
#include "../../event/events/guiTransformEvent.hpp"
#include "../../event/events/guiLightEvent.hpp"
#include "../../rendering/material/material.hpp"

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

void GuiPanels::renderDebugViewsPanel(
	const Entity& entity,
	EventBus& eventBus,
	std::vector<EntityState>& entityStates) {
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

	isDirty |= Ui::dragFloat3("Direction", entityState.light.direction, 0.01f, 100);
	isDirty |= Ui::colorField3("Ambient", entityState.light.ambient, 0.01f, 100);
	isDirty |= Ui::colorField3("Diffuse", entityState.light.diffuse, 0.01f, 100);
	isDirty |= Ui::colorField3("Specular", entityState.light.specular, 0.01f, 100);
	isDirty |= Ui::sliderFloat("Intensity", &entityState.light.intensity, 100.0, 1.0, 30.0);

	if (isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;

		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			matIdx,
			0u,
			entityState.light.direction,
			entityState.light.position,
			entityState.light.ambient,
			entityState.light.diffuse,
			entityState.light.specular,
			entityState.light.constant,
			entityState.light.linear,
			entityState.light.quadratic,
			entityState.light.cutOff,
			entityState.light.outerCutOff,
			entityState.light.intensity,
			entityState.light.castShadow);
	}
}

void GuiPanels::renderPointLight(const Entity& entity, EventBus& eventBus, EntityState& entityState) {
	bool isDirty = entityState.isDirty;

	isDirty |= Ui::colorField3("Ambient", entityState.light.ambient, 0.01f, 100);
	isDirty |= Ui::colorField3("Diffuse", entityState.light.diffuse, 0.01f, 100);
	isDirty |= Ui::colorField3("Specular", entityState.light.specular, 0.01f, 100);
	isDirty |= Ui::dragFloat("Constant", &entityState.light.constant, 0.01f, 100);
	isDirty |= Ui::dragFloat("Linear", &entityState.light.linear, 0.01f, 100);
	isDirty |= Ui::dragFloat("Quadratic", &entityState.light.quadratic, 0.01f, 100);
	isDirty |= Ui::sliderFloat("Intensity", &entityState.light.intensity, 100.0, 1.0, 30.0);
	isDirty |= ImGui::Checkbox("Cast Shadow", &entityState.light.castShadow);

	if (isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;
		uint32_t lightIdx = entity.getComponent<PointLightComponent>().idx;

		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			matIdx,
			lightIdx,
			entityState.light.direction,
			entityState.light.position,
			entityState.light.ambient,
			entityState.light.diffuse,
			entityState.light.specular,
			entityState.light.constant,
			entityState.light.linear,
			entityState.light.quadratic,
			entityState.light.cutOff,
			entityState.light.outerCutOff,
			entityState.light.intensity,
			entityState.light.castShadow);
	}
}

void GuiPanels::renderSpotLight(const Entity& entity, EventBus& eventBus, EntityState& entityState) {
	bool isDirty = entityState.isDirty;

	isDirty |= Ui::dragFloat3("Direction", entityState.light.direction, 0.01f, 100);
	isDirty |= Ui::colorField3("Ambient", entityState.light.ambient, 0.01f, 100);
	isDirty |= Ui::colorField3("Diffuse", entityState.light.diffuse, 0.01f, 100);
	isDirty |= Ui::colorField3("Specular", entityState.light.specular, 0.01f, 100);
	isDirty |= Ui::dragFloat("Constant", &entityState.light.constant, 0.01f, 100);
	isDirty |= Ui::dragFloat("Linear", &entityState.light.linear, 0.01f, 100);
	isDirty |= Ui::dragFloat("Quadratic", &entityState.light.quadratic, 0.01f, 100);
	isDirty |= Ui::dragFloat("Cutoff", &entityState.light.cutOff, 0.01f, 100);
	isDirty |= Ui::dragFloat("OuterCutoff", &entityState.light.outerCutOff, 0.01f, 100);
	isDirty |= Ui::sliderFloat("Intensity", &entityState.light.intensity, 100.0, 1.0, 30.0);
	isDirty |= ImGui::Checkbox("Cast Shadow", &entityState.light.castShadow);

	if (isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;
		uint32_t lightIdx = entity.getComponent<SpotLightComponent>().idx;

		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			matIdx,
			lightIdx,
			entityState.light.direction,
			entityState.light.position,
			entityState.light.ambient,
			entityState.light.diffuse,
			entityState.light.specular,
			entityState.light.constant,
			entityState.light.linear,
			entityState.light.quadratic,
			entityState.light.cutOff,
			entityState.light.outerCutOff,
			entityState.light.intensity,
			entityState.light.castShadow);
	}
}

void GuiPanels::renderPostProcessPanel(EventBus& eventBus) {
	struct Effect {
		const char* name{};
		bool enabled{};
		float exposure{};
		float intensity{};

		bool (* renderExtraControls)(Effect&){nullptr};
	} static effects[] = {
		{"Bloom", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Tone Mapping", false, 1.1f, 0.01f, [](Effect& self) { return Ui::sliderFloat("Exposure", &self.exposure, 100.0f); }},
		{"Grayscale", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Sepia", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Blur", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Edge Detection", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Sharpen", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Chromatic Aberration", false, 1.1f, 0.01f,[](Effect& self) { return Ui::sliderFloat("Intensity", &self.intensity, 100.0f, 0.01f, 0.1f); }},
		{"Gamma Correction", true, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"FXAA", false, 1.1f, 0.01f, [](Effect& self) { return false; }}
	};

	if (ImGui::Begin("Post-Processing")) {
		std::span<Effect> effectsSpan{effects};

		for (uint32_t i = 0; i < effectsSpan.size(); ++i) {
			auto& fx = effectsSpan[i];

			bool isDirty = ImGui::Checkbox(fx.name, &fx.enabled);
			isDirty |= fx.renderExtraControls(fx);

			if (isDirty) {
				eventBus.emitEvent<GuiPostProcessEvent>(i, fx.enabled, fx.exposure, fx.intensity);
			}
		}
	}
	ImGui::End();
}
