#pragma once
#include "../ECS/system.hpp"

struct RenderContext;
class UniformBuffer;
struct PointLightComponent;
struct DirectionalLightComponent;
struct SpotLightComponent;

class LightSystem final : public System {
public:
	explicit LightSystem(const RenderContext& ctx);

	[[nodiscard]] const UniformBuffer& getLightUBO() const { return *mLightUBO; }

	[[nodiscard]] const std::vector<PointLightComponent*>& getPointLights() const  { return pointLights; }
	[[nodiscard]] const std::vector<DirectionalLightComponent*>& getDirLights() const { return dirLights; }
	[[nodiscard]] const std::vector<SpotLightComponent*>& getSpotLights() const { return spotLights; }

	void update(const RenderContext& ctx);

private:
	void updateLightUBO(const RenderContext& ctx) const;

	std::unique_ptr<UniformBuffer> mLightUBO;
	std::vector<DirectionalLightComponent*> dirLights;
	std::vector<PointLightComponent*> pointLights;
	std::vector<SpotLightComponent*> spotLights;
};
