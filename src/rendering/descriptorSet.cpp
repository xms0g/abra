#include "descriptorSet.h"

DescriptorSet& DescriptorSet::write(const uint32_t binding, const TextureView texture) {
	mDescriptors.push_back({.type = DescriptorType::SampledImage, .binding = binding, .resource = texture});
	return *this;
}

const std::vector<Descriptor>& DescriptorSet::descriptors() const {
	return mDescriptors;
}
