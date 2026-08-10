#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <span>
#include "texture/texture.h"

enum class DescriptorType : uint32_t {
	UniformBuffer,
	SampledImage
};

struct Descriptor {
	DescriptorType type{};
	uint32_t binding{};
	std::variant<TextureView> resource{};
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

	DescriptorSet& write(uint32_t binding, TextureView texture);
	//void write(uint32_t binding, BufferView buffer);

	[[nodiscard]]
	const std::vector<Descriptor>& descriptors() const;

private:
	std::vector<Descriptor> mDescriptors;
};
