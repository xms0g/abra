#include "descriptorSet.h"

TextureView DescriptorSet::texture(const uint32_t binding) {
	return mDescriptors[binding].rs<TextureView>();
}

DescriptorSet& DescriptorSet::write(const uint32_t binding, const TextureView texture) {
	mDescriptors[binding] = {.type = DescriptorType::SampledImage, .binding = binding, .resource = texture};
	return *this;
}

DescriptorSet& DescriptorSet::write(const uint32_t binding, BufferView buffer) {
	mDescriptors[binding] =  {.type = DescriptorType::UniformBuffer, .binding = binding, .resource = buffer};
	return *this;
}

const std::array<Descriptor, MAX_DESCRIPTOR_SETS>& DescriptorSet::descriptors() const {
	return mDescriptors;
}
