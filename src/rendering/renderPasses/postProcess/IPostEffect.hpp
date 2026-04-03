#pragma once
#include <memory>
#include <string>

class FrameBuffer;
using RenderTargetType = std::array<std::unique_ptr<FrameBuffer>, 2>;

class IPostEffect {
public:
	IPostEffect() = default;

	IPostEffect(std::string n, const bool e) : mName(std::move(n)), mEnabled(e) {
	}

	virtual ~IPostEffect() = default;

	[[nodiscard]] const std::string& name() const { return mName; }

	[[nodiscard]] bool& enabled() { return mEnabled; }

	virtual uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		RenderTargetType& renderTargets) const = 0;

private:
	std::string mName;
	bool mEnabled{false};
};
