#include "descriptorSet.h"
#include <cassert>
#include "buffers/buffer.hpp"

DescriptorSet& DescriptorSet::write(const Descriptor& descriptor) {
	assert(mCount < MAX_DESCRIPTOR_COUNT);
	mDescriptors[mCount++] = descriptor;
	return *this;
}

DescriptorSet& DescriptorSet::write(const GPUTexture& texture) {
	assert(mCount < MAX_DESCRIPTOR_COUNT);
	mDescriptors[mCount++] = {.resource = texture};
	return *this;
}

DescriptorSet& DescriptorSet::write(const Buffer& buffer) {
	assert(mCount < MAX_DESCRIPTOR_COUNT);
	mDescriptors[mCount++] = {.resource = buffer};
	return *this;
}

const Descriptor& DescriptorSet::descriptor(const uint32_t index) const {
	return mDescriptors[index];
}

const Descriptor& DescriptorSet::operator[](const uint32_t index) const {
	return mDescriptors[index];
}
