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
		const auto& debug = entity.getComponent<DebugComponent>();
		static constexpr const char* modes[] = {"None", "Normals", "Wireframe"};

		static int32_t currentMode = static_cast<int32_t>(debug.mode);

		ImGui::Text("Mode");
		ImGui::SameLine(70);
		if (ImGui::Combo("##mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
			eventBus.emitEvent<GuiDebugEvent>(entity.id(), static_cast<DebugMode>(currentMode));
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderLightPanel(const Entity& entity, EventBus& eventBus) {
	ImGui::PushID(static_cast<int>(entity.id()));

	if (entity.hasComponent<DirectionalLightComponent>()) {
		renderDirLight(entity, eventBus);
	} else if (entity.hasComponent<SpotLightComponent>()) {
		renderSpotLight(entity, eventBus);
	} else if (entity.hasComponent<PointLightComponent>()) {
		renderPointLight(entity, eventBus);
	}

	ImGui::PopID();
}

void GuiPanels::renderDirLight(const Entity& entity, EventBus& eventBus) {
	bool isDirty{false};
	auto& dirLight = entity.getComponent<DirectionalLightComponent>();

	isDirty |= ui::dragFloat3("Direction", dirLight.direction, 0.01f, 100);
	isDirty |= ui::colorField3("Ambient", dirLight.ambient, 0.01f, 100);
	isDirty |= ui::colorField3("Diffuse", dirLight.diffuse, 0.01f, 100);
	isDirty |= ui::colorField3("Specular", dirLight.specular, 0.01f, 100);
	isDirty |= ui::sliderFloat("Intensity", &dirLight.intensity, 100.0, 1.0, 30.0);

	if (isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;

		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			matIdx,
			0u,
			dirLight.direction,
			glm::vec3(0.0f),
			dirLight.ambient,
			dirLight.diffuse,
			dirLight.specular,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			dirLight.intensity,
			true);
	}
}

void GuiPanels::renderPointLight(const Entity& entity, EventBus& eventBus) {
	auto& transform = entity.getComponent<TransformComponent>();
	auto& pointLight = entity.getComponent<PointLightComponent>();

	transform.isDirty |= ui::colorField3("Ambient", pointLight.ambient, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Diffuse", pointLight.diffuse, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Specular", pointLight.specular, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Constant", &pointLight.constant, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Linear", &pointLight.linear, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Quadratic", &pointLight.quadratic, 0.01f, 100);
	transform.isDirty |= ui::sliderFloat("Intensity", &pointLight.intensity, 100.0, 1.0, 30.0);
	transform.isDirty |= ImGui::Checkbox("Cast Shadow", &pointLight.castShadow);

	if (transform.isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;

		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			matIdx,
			pointLight.idx,
			glm::vec3(0.0f),
			transform.position,
			pointLight.ambient,
			pointLight.diffuse,
			pointLight.specular,
			pointLight.constant,
			pointLight.linear,
			pointLight.quadratic,
			0.0f,
			0.0f,
			pointLight.intensity,
			pointLight.castShadow);
	}

	transform.isDirty = false;
}

void GuiPanels::renderSpotLight(const Entity& entity, EventBus& eventBus) {
	auto& transform = entity.getComponent<TransformComponent>();
	auto& spotLight = entity.getComponent<SpotLightComponent>();

	transform.isDirty |= ui::dragFloat3("Direction", spotLight.direction, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Ambient", spotLight.ambient, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Diffuse", spotLight.diffuse, 0.01f, 100);
	transform.isDirty |= ui::colorField3("Specular", spotLight.specular, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Constant", &spotLight.constant, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Linear", &spotLight.linear, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Quadratic", &spotLight.quadratic, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("Cutoff", &spotLight.cutOff, 0.01f, 100);
	transform.isDirty |= ui::dragFloat("OuterCutoff", &spotLight.outerCutOff, 0.01f, 100);
	transform.isDirty |= ui::sliderFloat("Intensity", &spotLight.intensity, 100.0, 1.0, 30.0);
	transform.isDirty |= ImGui::Checkbox("Cast Shadow", &spotLight.castShadow);

	if (transform.isDirty) {
		uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;

		eventBus.emitEvent<GuiLightEvent>(
			entity.id(),
			matIdx,
			spotLight.idx,
			spotLight.direction,
			transform.position,
			spotLight.ambient,
			spotLight.diffuse,
			spotLight.specular,
			spotLight.constant,
			spotLight.linear,
			spotLight.quadratic,
			spotLight.cutOff,
			spotLight.outerCutOff,
			spotLight.intensity,
			spotLight.castShadow);
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
		std::span<Effect> effectsSpan{effects};

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
