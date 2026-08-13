#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <span>
#include "texture/texture.h"

#define MAX_DESCRIPTOR_SETS 32

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
	DescriptorType type{};
	uint32_t binding{};
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

	TextureView texture(uint32_t binding);

	DescriptorSet& write(uint32_t binding, TextureView texture);

	DescriptorSet& write(uint32_t binding, BufferView buffer);

	[[nodiscard]]
	const std::array<Descriptor, MAX_DESCRIPTOR_SETS>& descriptors() const;

private:
	std::array<Descriptor, MAX_DESCRIPTOR_SETS> mDescriptors;
};
