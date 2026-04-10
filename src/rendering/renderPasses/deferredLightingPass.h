#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "IRenderPass.hpp"
#include "../models/quad.h"
#include "../models/cube.h"

class CubemapBuffer;
class FrameBuffer;
class Shader;

class DeferredLightingPass final : public IRenderPass {
public:
	~DeferredLightingPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	void createEnvMap(const RenderContext& ctx);

	void createIrradianceMap();

	void createPrefilterMap(const RenderContext& ctx);

	void createBrdfLUT() const;

	std::unique_ptr<Shader> mShader;
	std::unique_ptr<Models::SingleQuad> mQuad;

	std::unique_ptr<CubemapBuffer> mEnvMapBuffer;
	std::unique_ptr<CubemapBuffer> mIrradianceMapBuffer;
	std::unique_ptr<CubemapBuffer> mPrefilterMapBuffer;
	std::unique_ptr<FrameBuffer> mBrdfLUTBuffer;

	Models::Cube cube;
	static constexpr uint32_t FACES = 6;

	glm::mat4 mCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 mCaptureViews[FACES] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

};
