#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <span>
#include "texture/texture.h"

#define MAX_DESCRIPTOR_COUNT 32

class Buffer;

enum class DescriptorType : uint32_t {
	UniformBuffer,
	SampledImage
};

struct Descriptor {
	using ResourceRef = std::variant<std::monostate, std::reference_wrapper<const GPUTexture>, std::reference_wrapper<const Buffer>>;
	ResourceRef resource;

	[[nodiscard]]
	bool isValid() const {
		return !std::holds_alternative<std::monostate>(resource);
	}

	template<typename T>
	const T& refRes() const {
		return std::get<std::reference_wrapper<const T>>(resource).get();
	}
};

struct DescriptorBinding {
	std::string name;
	DescriptorType type{};
	int32_t binding{};
};

struct DescriptorSetLayout {
	std::vector<DescriptorBinding> bindings;
};

class DescriptorSet {
public:
	DescriptorSet& write(const Descriptor& descriptor);

	DescriptorSet& write(const GPUTexture& texture);

	DescriptorSet& write(const Buffer& buffer);

	[[nodiscard]]
	const Descriptor& descriptor(uint32_t index) const;

	const Descriptor& operator[](uint32_t index) const;

private:
	std::array<Descriptor, MAX_DESCRIPTOR_COUNT> mDescriptors{};
	uint32_t mCount{0};
};
