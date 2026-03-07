#pragma once
#include <memory>
#include <string>

class FrameBuffer;

class IPostEffect {
public:
	IPostEffect() = default;

	IPostEffect(std::string n, const bool e) : mName(std::move(n)), mEnabled(e) {
	}

	virtual ~IPostEffect() = default;

	[[nodiscard]] const std::string& name() const { return mName; }

	[[nodiscard]] bool& enabled() { return mEnabled; }

	virtual uint32_t render(uint32_t sceneTexture,
	                        uint32_t VAO,
	                        int& toggle,
	                        const std::unique_ptr<FrameBuffer>* renderTargets) const = 0;

private:
	std::string mName;
	bool mEnabled{false};
};
