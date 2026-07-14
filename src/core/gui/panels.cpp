#include "panels.h"
#include <span>
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "ui.h"
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
	if (ImGui::TreeNodeEx("Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("%s FPS", std::to_string(fps).c_str());
		ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
		ImGui::Text("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		ImGui::Text("OpenGL Driver Vendor: %s", glGetString(GL_VENDOR));
		ImGui::Text("OpenGL Renderer: %s", glGetString(GL_RENDERER));

		ImGui::TreePop();
	}
}

void GuiPanels::renderTransformPanel(const Entity& entity, EventBus& eventBus) {
	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& transform = entity.getComponent<TransformComponent>();

		transform.isDirty |= ui::dragFloat3("Position", transform.position);
		transform.isDirty |= ui::dragFloat3("Rotation", transform.rotation, 1.0f);
		transform.isDirty |= ui::dragFloat3("Scale", transform.scale);

		if (transform.isDirty) {
			eventBus.emitEvent<GuiTransformEvent>(entity.id(), transform.position, transform.rotation, transform.scale);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderDebugViewsPanel(const Entity& entity, EventBus& eventBus) {
	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Debug Views", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& debug = entity.getComponent<DebugComponent>();
		static constexpr const char* modes[] = {"None", "Normals", "Wireframe"};

		int32_t currentMode = static_cast<int32_t>(debug.mode);

		ImGui::Text("Mode");
		ImGui::SameLine(70);
		if (ImGui::Combo("##mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
			debug.mode = static_cast<DebugMode>(currentMode);
			eventBus.emitEvent<GuiDebugEvent>(entity.id(), debug.mode);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderLightPanel(const Entity& entity, EventBus& eventBus) {
	ImGui::PushID(static_cast<int>(entity.id()));

	if (auto dirlight = entity.tryGetComponent<DirectionalLightComponent>()) {
		renderDirLight(entity,*dirlight, eventBus);
	} else if (auto pointlight = entity.tryGetComponent<PointLightComponent>()) {
		renderPointLight(entity, *pointlight, eventBus);
	} else if (auto spotlight = entity.tryGetComponent<SpotLightComponent>()) {
		renderSpotLight(entity, *spotlight, eventBus);
	}

	ImGui::PopID();
}

void GuiPanels::renderDirLight(const Entity& entity, DirectionalLightComponent& dirlight, EventBus& eventBus) {
	bool isDirty{false};

	isDirty |= ui::dragFloat3("Direction", dirlight.direction, 0.01f, 100);
	isDirty |= ui::colorField3("Ambient", dirlight.ambient, 0.01f, 100);
	isDirty |= ui::colorField3("Diffuse", dirlight.diffuse, 0.01f, 100);
	isDirty |= ui::colorField3("Specular", dirlight.specular, 0.01f, 100);
	isDirty |= ui::sliderFloat("Intensity", &dirlight.intensity, 100.0, 1.0, 30.0);

	if (isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;

		eventBus.emitEvent<GuiLightEvent>(entity.id(), matIdx, 0u);
	}
}

void GuiPanels::renderPointLight(const Entity& entity, PointLightComponent& pointlight, EventBus& eventBus) {
	static auto& transform = entity.getComponent<TransformComponent>();

	transform.isDirty |= ui::colorField3("Ambient", pointlight.ambient, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Diffuse", pointlight.diffuse, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Specular", pointlight.specular, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Constant", &pointlight.constant, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Linear", &pointlight.linear, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Quadratic", &pointlight.quadratic, 0.01f, 100);
	transform.isDirty |= ui::sliderFloat("Intensity", &pointlight.intensity, 100.0, 1.0, 30.0);
	transform.isDirty |= ImGui::Checkbox("Cast Shadow", &pointlight.castShadow);

	if (transform.isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;
		pointlight.position = transform.position;

		eventBus.emitEvent<GuiLightEvent>(entity.id(), matIdx, pointlight.idx);
	}

	transform.isDirty = false;
}

void GuiPanels::renderSpotLight(const Entity& entity, SpotLightComponent& spotlight, EventBus& eventBus) {
	static auto& transform = entity.getComponent<TransformComponent>();

	transform.isDirty |= ui::dragFloat3("Direction", spotlight.direction, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Ambient", spotlight.ambient, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Diffuse", spotlight.diffuse, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Specular", spotlight.specular, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Constant", &spotlight.constant, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Linear", &spotlight.linear, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Quadratic", &spotlight.quadratic, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Cutoff", &spotlight.cutOff, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("OuterCutoff", &spotlight.outerCutOff, 0.01f, 100);
	transform.isDirty |= ui::sliderFloat("Intensity", &spotlight.intensity, 100.0, 1.0, 30.0);
	transform.isDirty |= ImGui::Checkbox("Cast Shadow", &spotlight.castShadow);

	if (transform.isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;
		spotlight.position = transform.position;

		eventBus.emitEvent<GuiLightEvent>(entity.id(), matIdx, spotlight.idx);
	}

	transform.isDirty = false;
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
		{"Tone Mapping", false, 1.1f, 0.01f, [](Effect& self) { return ui::sliderFloat("Exposure", &self.exposure, 100.0f); }},
		{"Grayscale", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Sepia", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Blur", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Edge Detection", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Sharpen", false, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"Chromatic Aberration", false, 1.1f, 0.01f,[](Effect& self) { return ui::sliderFloat("Intensity", &self.intensity, 100.0f, 0.01f, 0.1f); }},
		{"Gamma Correction", true, 1.1f, 0.01f, [](Effect& self) { return false; }},
		{"FXAA", false, 1.1f, 0.01f, [](Effect& self) { return false; }}
	};

	if (ImGui::TreeNodeEx("Post-Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
		static std::span<Effect> effectsSpan{effects};

		for (uint32_t i = 0; i < effectsSpan.size(); ++i) {
			auto& fx = effectsSpan[i];

			bool isDirty = ImGui::Checkbox(fx.name, &fx.enabled);
			isDirty |= fx.renderExtraControls(fx);

			if (isDirty) {
				eventBus.emitEvent<GuiPostProcessEvent>(i, fx.enabled, fx.exposure, fx.intensity);
			}
		}
		ImGui::TreePop();
	}
}
