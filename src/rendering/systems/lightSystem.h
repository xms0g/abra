#pragma once
#include "glm/glm.hpp"
#include "../../ECS/system.hpp"
#include "../buffers/uniformBuffer.h"

class GraphicsEncoder;
struct RenderContext;
struct GuiLightEvent;
class EventBus;
struct PointLightComponent;
struct DirectionalLightComponent;
struct SpotLightComponent;

class LightSystem final : public System {
public:
	LightSystem();

	void configure(RenderContext& ctx, GraphicsEncoder& encoder, EventBus& eventBus);

private:
	void updateLightUBO() const;

	void onGuiUpdate(const GuiLightEvent& event);

	EventBus* mEventBus{};
	RenderContext* mCtx{};
	UniformBuffer mUBO;
	std::vector<DirectionalLightComponent*> mDirLights{};
	std::vector<PointLightComponent*> mPointLights{};
	std::vector<SpotLightComponent*> mSpotLights{};

	struct alignas(16) DirectionalLight {
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 intensity;
	};

	struct alignas(16) PointLight {
		glm::vec4 position;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 attenuation;
		glm::vec4 intensity;
	};

	struct alignas(16) SpotLight {
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 attenuation;
		glm::vec4 cutOff;
	};

	struct alignas(16) UniformBufferObject {
		DirectionalLight dirLights[1]{};
		PointLight pointLights[4]{};
		SpotLight spotLights[4]{};
		glm::ivec4 lightCount{};
	};
};
