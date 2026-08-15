#include "descriptorSet.h"
#include <cassert>

TextureView DescriptorSet::texture(const uint32_t index) {
	return mDescriptors[index].rs<TextureView>();
}

DescriptorSet& DescriptorSet::write(const TextureView texture) {
	assert(mCount < MAX_DESCRIPTOR_SETS);
	mDescriptors[mCount++] = {.resource = texture};
	return *this;
}

DescriptorSet& DescriptorSet::write(BufferView buffer) {
	assert(mCount < MAX_DESCRIPTOR_SETS);
	mDescriptors[mCount++] =  {.resource = buffer};
	return *this;
}

const Descriptor& DescriptorSet::descriptor(const uint32_t index) const {
	return mDescriptors[index];
}
