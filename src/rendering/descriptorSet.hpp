#pragma once
#include <variant>
#include <vector>
#include <span>
#include "texture.hpp"

#define MAX_DESCRIPTOR_COUNT 32

enum class DescriptorType : uint32_t {
	UniformBuffer,
	SampledImage
};

class GPUBuffer;

struct Descriptor {
	using ResourceRef = std::variant<std::reference_wrapper<const GPUTexture>, std::reference_wrapper<const GPUBuffer>>;
	ResourceRef resource;

	template<typename T>
	const T& resRef() const {
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
	[[nodiscard]]
	bool contains(uint32_t binding) const;

	DescriptorSet& write(uint32_t binding, const Descriptor& descriptor);

	DescriptorSet& write(uint32_t binding, const GPUTexture& texture);

	DescriptorSet& write(uint32_t binding, const GPUBuffer& buffer);

	const Descriptor& operator[](uint32_t binding) const;

private:
	std::array<std::optional<Descriptor>, MAX_DESCRIPTOR_COUNT> mDescriptors{};
};
