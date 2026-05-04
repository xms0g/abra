#pragma once
#include <memory>
#include <string>

class FrameBuffer;
using PingPongBuffer = std::array<std::unique_ptr<FrameBuffer>, 2>;

class IPostEffect {
public:
	IPostEffect() = default;

	IPostEffect(std::string n, const bool e) : mName(std::move(n)), mEnabled(e) {
	}

	virtual ~IPostEffect() = default;

	[[nodiscard]]
	const std::string& name() const { return mName; }

	[[nodiscard]]
	bool enabled() const { return mEnabled; }

	void enabled(const bool e) { mEnabled = e; }

	virtual uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		PingPongBuffer& pingPong) const = 0;

private:
	std::string mName;
	bool mEnabled{false};
};
