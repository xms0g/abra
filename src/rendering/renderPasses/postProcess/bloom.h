#pragma once
#include "IPostEffect.hpp"

class Shader;
class FrameBuffer;

class Bloom final : public IPostEffect {
public:
	Bloom(const std::string& name, int width, int height, bool enabled = false);

	~Bloom() override;

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		int& toggle,
		FrameBuffer** renderTargets) const override;

private:
	uint32_t brightFilterPass(uint32_t sceneTexture, uint32_t VAO, int& toggle) const;

	uint32_t blurPass(uint32_t sceneTexture, uint32_t VAO, int& toggle) const;

	[[nodiscard]] uint32_t combinePass(
		uint32_t sceneTexture,
		uint32_t bloomBlur,
		uint32_t vao,
		const int& toggle) const;

	FrameBuffer* mRenderTargets[2]{};
	std::unique_ptr<Shader> brightFilter;
	std::unique_ptr<Shader> blur;
	std::unique_ptr<Shader> combine;
};
