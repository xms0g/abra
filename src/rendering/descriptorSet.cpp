#include "descriptorSet.hpp"
#include <cassert>
#include "buffers/buffer.hpp"

bool DescriptorSet::contains(const uint32_t binding) const {
	return binding < MAX_DESCRIPTOR_COUNT && mDescriptors[binding].has_value();
}

DescriptorSet& DescriptorSet::write(const uint32_t binding, const Descriptor& descriptor) {
	assert(binding < MAX_DESCRIPTOR_COUNT);
	mDescriptors[binding] = descriptor;
	return *this;
}

DescriptorSet& DescriptorSet::write(const uint32_t binding, const GPUTexture& texture) {
	assert(binding < MAX_DESCRIPTOR_COUNT);
	mDescriptors[binding] = {.resource = texture};
	return *this;
}

DescriptorSet& DescriptorSet::write(const uint32_t binding, const GPUBuffer& buffer) {
	assert(binding < MAX_DESCRIPTOR_COUNT);
	mDescriptors[binding] = {.resource = buffer};
	return *this;
}

const Descriptor& DescriptorSet::operator[](const uint32_t binding) const {
	return mDescriptors[binding].value();
}
