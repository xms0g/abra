#pragma once
#include <vector>
#include <memory>
#include "directionalShadowPass.h"
#include "omnidirectionalShadowPass.h"
#include "perspectiveShadowPass.h"
#include "../IRenderPass.hpp"
#include "../../../config/config.hpp"

struct RenderContext;
class UniformBuffer;

class ShadowPass final: public IRenderPass {
public:
	~ShadowPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

	[[nodiscard]] const std::array<uint32_t, 3>& getShadowMaps() const;

	[[nodiscard]] const UniformBuffer* getShadowUBO() const;

private:
	void directionalShadowPass(const RenderContext& ctx);

	void omnidirectionalShadowPass(const RenderContext& ctx);

	void perspectiveShadowPass(const RenderContext& ctx);

	struct alignas(16) ShadowData {
		glm::mat4 lightSpaceMatrix;
		glm::mat4 persLightSpaceMatrix[MAX_SPOT_LIGHTS];
		glm::vec4 omniFarPlanes;
	};

	ShadowData mShadowData{};

	std::array<uint32_t, 3> mShadowMaps{};
	std::unique_ptr<DirectionalShadowPass> mDirShadowPass;
	std::unique_ptr<OmnidirectionalShadowPass> mOmnidirShadowPass;
	std::unique_ptr<PerspectiveShadowPass> mPerspectiveShadowPass;
	std::unique_ptr<UniformBuffer> mShadowUBO;
};
