#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <span>
#include "texture/texture.h"

#define MAX_DESCRIPTOR_COUNT 32

enum class DescriptorType : uint32_t {
	None,
	UniformBuffer,
	SampledImage
};

struct BufferView {
	uint32_t id{0};
	uint32_t target{0};
	int32_t size{0};
};

struct Descriptor {
	std::variant<TextureView, BufferView> resource{};

	template<typename T>
	T rs() {
		return std::get<T>(resource);
	}
};

struct DescriptorBinding {
	std::string name;
	DescriptorType type;
	int32_t binding{};
};

struct DescriptorSetLayout {
	std::vector<DescriptorBinding> bindings;
};

class DescriptorSet {
public:
	DescriptorSet() = default;

	DescriptorSet& write(const Descriptor& descriptor);

	DescriptorSet& write(TextureView texture);

	DescriptorSet& write(BufferView buffer);

	[[nodiscard]]
	const Descriptor& descriptor(uint32_t index) const;

	const Descriptor& operator[](uint32_t index) const;

private:
	std::array<Descriptor, MAX_DESCRIPTOR_COUNT> mDescriptors;
	uint32_t mCount{0};
};
