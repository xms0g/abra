#include "descriptorSet.h"

DescriptorSet& DescriptorSet::write(const uint32_t binding, const TextureView texture) {
	mDescriptors.push_back({.type = fromTextureTarget(texture.target), .binding = binding, .resource = texture});
	return *this;
}

const std::vector<Descriptor>& DescriptorSet::descriptors() const {
	return mDescriptors;
}


DescriptorType DescriptorSet::fromTextureTarget(const TextureTarget target) {
	switch (target) {
		case TextureTarget::Texture2D:
			return DescriptorType::Sampler2D;
		case TextureTarget::Texture2DMultisample:
			return DescriptorType::Sampler2D;
		case TextureTarget::Texture2DArray:
			return DescriptorType::Sampler2DArray;
		case TextureTarget::TextureCubeMap:
			return DescriptorType::SamplerCube;
		case TextureTarget::TextureCubeMapArray:
			return DescriptorType::SamplerCubeArray;
	}

	return DescriptorType::Sampler2D;
}
