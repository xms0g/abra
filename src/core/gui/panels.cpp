#include "panels.h"
#include <span>
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "ui.h"
#include "../../ECS/registry.h"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiPostProcessEvent.hpp"

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

void GuiPanels::renderTransformPanel(const Entity& entity, TransformComponent& transform, bool& transformChanged) {
	ImGui::PushID(static_cast<int>(entity.id()));
	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		transformChanged |= ui::dragFloat3("Position", transform.position);
		transformChanged |= ui::dragFloat3("Rotation", transform.rotation, 1.0f);
		transformChanged |= ui::dragFloat3("Scale", transform.scale);

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void GuiPanels::renderDebugViewsPanel(const Entity& entity, EventBus& eventBus) {
	if (const auto debug = entity.tryGetComponent<DebugComponent>()) {
		ImGui::PushID(static_cast<int>(entity.id()));
		if (ImGui::TreeNodeEx("Debug Views", ImGuiTreeNodeFlags_DefaultOpen)) {
			static constexpr const char* modes[] = {"None", "Normals", "Wireframe"};

			int32_t currentMode = static_cast<int32_t>(debug->mode);

			ImGui::Text("Mode");
			ImGui::SameLine(70);
			if (ImGui::Combo("##mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
				debug->mode = static_cast<DebugMode>(currentMode);
				eventBus.emitEvent<GuiDebugEvent>(entity.id(), debug->mode);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void GuiPanels::renderLightPanel(const Entity& entity, bool& lightChanged, uint32_t& lightIdx) {
	ImGui::PushID(static_cast<int>(entity.id()));

	if (auto dirlight = entity.tryGetComponent<DirectionalLightComponent>()) {
		renderDirLight(*dirlight, lightChanged, lightIdx);
	}

	if (auto pointlight = entity.tryGetComponent<PointLightComponent>()) {
		renderPointLight(*pointlight, lightChanged, lightIdx);
	}

	if (auto spotlight = entity.tryGetComponent<SpotLightComponent>()) {
		renderSpotLight(*spotlight, lightChanged, lightIdx);
	}

	ImGui::PopID();
}

void GuiPanels::renderDirLight(DirectionalLightComponent& dirlight,  bool& lightChanged, uint32_t& lightIdx) {
	lightChanged |= ui::dragFloat3("Direction", dirlight.direction, 0.01f, 100);
	lightChanged |= ui::colorField3("Ambient", dirlight.ambient, 0.01f, 100);
	lightChanged |= ui::colorField3("Diffuse", dirlight.diffuse, 0.01f, 100);
	lightChanged |= ui::colorField3("Specular", dirlight.specular, 0.01f, 100);
	lightChanged |= ui::sliderFloat("Intensity", &dirlight.intensity, 100.0, 1.0, 30.0);

	lightIdx = 0u;
}

void GuiPanels::renderPointLight(PointLightComponent& pointlight,  bool& lightChanged, uint32_t& lightIdx) {
	lightChanged |= ui::colorField3("Ambient", pointlight.ambient, 0.01f, 100);
	lightChanged |= ui::colorField3("Diffuse", pointlight.diffuse, 0.01f, 100);
	lightChanged |= ui::colorField3("Specular", pointlight.specular, 0.01f, 100);
	lightChanged |= ui::dragFloat("Constant", &pointlight.constant, 0.01f, 100);
	lightChanged |= ui::dragFloat("Linear", &pointlight.linear, 0.01f, 100);
	lightChanged |= ui::dragFloat("Quadratic", &pointlight.quadratic, 0.01f, 100);
	lightChanged |= ui::sliderFloat("Intensity", &pointlight.intensity, 100.0, 1.0, 30.0);
	lightChanged |= ImGui::Checkbox("Cast Shadow", &pointlight.castShadow);

	lightIdx = pointlight.idx;
}

void GuiPanels::renderSpotLight(SpotLightComponent& spotlight,  bool& lightChanged, uint32_t& lightIdx) {
	lightChanged |= ui::dragFloat3("Direction", spotlight.direction, 0.01f, 100);
	lightChanged |= ui::colorField3("Ambient", spotlight.ambient, 0.01f, 100);
	lightChanged |= ui::colorField3("Diffuse", spotlight.diffuse, 0.01f, 100);
	lightChanged |= ui::colorField3("Specular", spotlight.specular, 0.01f, 100);
	lightChanged |= ui::dragFloat("Constant", &spotlight.constant, 0.01f, 100);
	lightChanged |= ui::dragFloat("Linear", &spotlight.linear, 0.01f, 100);
	lightChanged |= ui::dragFloat("Quadratic", &spotlight.quadratic, 0.01f, 100);
	lightChanged |= ui::dragFloat("Cutoff", &spotlight.cutOff, 0.01f, 100);
	lightChanged |= ui::dragFloat("OuterCutoff", &spotlight.outerCutOff, 0.01f, 100);
	lightChanged |= ui::sliderFloat("Intensity", &spotlight.intensity, 100.0, 1.0, 30.0);
	lightChanged |= ImGui::Checkbox("Cast Shadow", &spotlight.castShadow);

	lightIdx = spotlight.idx;
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

			bool lightChanged = ImGui::Checkbox(fx.name, &fx.enabled);
			lightChanged |= fx.renderExtraControls(fx);

			if (lightChanged) {
				eventBus.emitEvent<GuiPostProcessEvent>(i, fx.enabled, fx.exposure, fx.intensity);
			}
		}
		ImGui::TreePop();
	}
}
