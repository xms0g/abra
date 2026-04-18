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

	[[nodiscard]]
	const UniformBuffer& ubo() const;

	[[nodiscard]]
	const std::vector<PointLightComponent*>& pointLights() const;

	[[nodiscard]]
	const std::vector<DirectionalLightComponent*>& dirLights() const;

	[[nodiscard]]
	const std::vector<SpotLightComponent*>& spotLights() const;

	void update(const RenderContext& ctx);

private:
	void updateLightUBO(const RenderContext& ctx) const;

	std::unique_ptr<UniformBuffer> mUBO;
	std::vector<DirectionalLightComponent*> mDirLights;
	std::vector<PointLightComponent*> mPointLights;
	std::vector<SpotLightComponent*> mSpotLights;
};
