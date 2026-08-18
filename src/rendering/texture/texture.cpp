#include "texture.h"
#include <iostream>
#include <utility>
#include "glad/glad.h"
#include "image/stb_image.h"
#include "../material/material.hpp"
#include "../glUtils.hpp"

Texture::Texture(const uint32_t id, const uint32_t type, const TextureTarget target, std::string path)
	: id(id),
      type(type),
      target(target),
      path(std::move(path)) {
}

Texture::~Texture() {
	if (id)
		glDeleteTextures(1, &id);
}

Texture::Texture(Texture&& other) noexcept
	: id(std::exchange(other.id, 0)),
	  type(std::exchange(other.type, 0)),
	  target(std::exchange(other.target, {})),
      path(std::move(other.path)) {
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (id)
			glDeleteTextures(1, &id);

		id = std::exchange(other.id, 0);
		type = std::exchange(other.type, 0);
		target = std::exchange(other.target, {});
		path = std::move(other.path);
	}

	return *this;
}

void Texture::uploadCubeFace(const uint32_t face,
							const void* data,
							const int32_t width,
							const int32_t height,
							const int32_t format,
							const int32_t internalFormat,
							const int32_t dataType) const {
	assert(target == TextureTarget::TextureCubeMap);
	assert(face < 6);

	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
		0,
		internalFormat,
		width,
		height,
		0,
		format,
		dataType,
		data
	);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::info(const std::string_view path, int32_t& width, int32_t& height) {
	int32_t channel;
	stbi_info(path.data(), &width, &height, &channel);
}

Texture Texture::load(const std::span<const std::string> paths, TextureConfig& config) {
	int32_t width, height, channel;

	switch (config.target) {
		case TextureTarget::Texture2D: {
			void* data = nullptr;
			const char* path = paths[0].c_str();

			if (config.isHDR) {
				stbi_set_flip_vertically_on_load(true);

				data = stbi_loadf(path, &width, &height, &channel, 0);
				if (!data) {
					throw std::runtime_error(std::format("HDR texture failed to load at path: {}", path));
				}

				config.width = width;
				config.height = height;
			} else {
				data = stbi_load(path, &width, &height, &channel, 4);
				if (!data) {
					throw std::runtime_error(std::format("Texture failed to load at path: {}", path));
				}

				config.width = width;
				config.height = height;
			}

			auto texture = generate(data, config);

			stbi_image_free(data);
			stbi_set_flip_vertically_on_load(false);
			return texture;
		}
		case TextureTarget::TextureCubeMap: {
			if (paths.size() != 6) {
				throw std::runtime_error("Cubemap texture must have 6 paths");
			}

			auto texture = generate(nullptr, config);

			for (uint32_t i = 0; i < 6; ++i) {
				unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channel, 0);

				if (!data) {
					throw std::runtime_error(std::format("Cubemap texture failed to load at path: {}", paths[i]));
				}

				texture.uploadCubeFace(i, data, width, height, toUnderlying(config.format), toUnderlying(config.internalFormat), toUnderlying(config.dataType));

				stbi_image_free(data);
			}

			return texture;
		}
	}

	return {};
}

Texture Texture::generate(const void* data, const TextureConfig& config) {
	uint32_t textureID;

	const auto target = toUnderlying(config.target);
	const auto format = toUnderlying(config.format);
	const auto internalFormat = toUnderlying(config.internalFormat);
	const auto dataType = toUnderlying(config.dataType);
	const auto wrapS = toUnderlying(config.parameters.wrapS);
	const auto wrapT = toUnderlying(config.parameters.wrapT);
	const auto wrapR = toUnderlying(config.parameters.wrapR);
	const auto minFilter = toUnderlying(config.parameters.minFilter);
	const auto magFilter = toUnderlying(config.parameters.magFilter);

	glGenTextures(1, &textureID);
	glBindTexture(target, textureID);

	switch (config.target) {
		case TextureTarget::Texture2D: {
			glTexImage2D(
				target,
				0,
				internalFormat,
				config.width,
				config.height,
				0,
				format,
				dataType,
				data);

			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);

			break;
		}
		case TextureTarget::Texture2DMultisample: {
			glTexImage2DMultisample(
				target,
				config.samples,
				internalFormat,
				config.width,
				config.height,
				GL_TRUE);
			break;
		}
		case TextureTarget::Texture2DArray: {
			glTexImage3D(
				target,
			0,
				internalFormat,
				config.width,
				config.height,
				config.layers,
				0,
				format,
				dataType,
				data);

			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);

			break;
		}
		case TextureTarget::TextureCubeMap: {
			for (uint32_t i = 0; i < 6; ++i) {
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					internalFormat,
					config.width,
					config.height,
					0,
					format,
					dataType,
					data);
			}

			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
			glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);

			break;
		}
		case TextureTarget::TextureCubeMapArray: {
			glTexImage3D(
				target,
				0,
				internalFormat,
				config.width,
				config.height,
				config.layers * 6,
				0,
				format,
				dataType,
				data);

			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
			glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);

			break;
		}
	}

	if (config.parameters.wrapS == TextureWrap::ClampToBorder ||
		config.parameters.wrapR == TextureWrap::ClampToBorder ||
		config.parameters.wrapT == TextureWrap::ClampToBorder) {
		constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	glBindTexture(target, 0);

	return {textureID, 0, config.target, ""};
}

Texture Texture::generateColorAttachment(const int width, const int height) {
	return generate(nullptr, {
		.target = TextureTarget::Texture2D,
		.internalFormat = InternalFormat::RGBA8,
		.format = BaseFormat::RGBA,
		.parameters = {
			.minFilter = TextureFilter::Linear,
			.magFilter = TextureFilter::Linear,
			.wrapS = TextureWrap::ClampToEdge,
			.wrapT = TextureWrap::ClampToEdge,
		},
		.dataType = DataType::UnsignedByte,
		.width = width,
		.height = height,
		.samples = 1,
		.layers = 1,
	});
}

Texture Texture::generateColorAttachmentMultisampled(const int width, const int height, const int samples) {
	return generate(nullptr, {
		.target = TextureTarget::Texture2DMultisample,
		.internalFormat = InternalFormat::RGBA8,
		.format = BaseFormat::RGBA,
		.dataType = DataType::UnsignedByte,
		.width = width,
		.height = height,
		.samples = samples,
		.layers = 1,
	});
}

Texture Texture::generateColorAttachmentFP(const int width, const int height) {
	return generate(nullptr, {
		.target = TextureTarget::Texture2D,
		.internalFormat = InternalFormat::RGBAFloat,
		.format = BaseFormat::RGBA,
		.parameters = {
			.minFilter = TextureFilter::Linear,
			.magFilter = TextureFilter::Linear,
			.wrapS = TextureWrap::ClampToEdge,
			.wrapT = TextureWrap::ClampToEdge,
		},
		.dataType = DataType::Float,
		.width = width,
		.height = height,
		.samples = 1,
		.layers = 1,
	});
}

Texture Texture::generateColorAttachmentFPMultisampled(const int width, const int height, int samples) {
	return generate(nullptr, {
		.target = TextureTarget::Texture2DMultisample,
		.internalFormat = InternalFormat::RGBAFloat,
		.format = BaseFormat::RGBA,
		.dataType = DataType::Float,
		.width = width,
		.height = height,
		.samples = samples,
		.layers = 1,
	});
}

Texture Texture::generateColorAttachmentCubemap(const int width, const int height) {
	return generate(nullptr, {
		.target = TextureTarget::TextureCubeMap,
		.internalFormat = InternalFormat::RGBFloat,
		.format = BaseFormat::RGB,
		.parameters = {
			.minFilter = TextureFilter::Linear,
			.magFilter = TextureFilter::Linear,
			.wrapS = TextureWrap::ClampToEdge,
			.wrapT = TextureWrap::ClampToEdge,
			.wrapR = TextureWrap::ClampToEdge,
		},
		.dataType = DataType::Float,
		.width = width,
		.height = height,
		.samples = 1,
		.layers = 1,
	});
}

Texture Texture::generateDepthAttachment(const int width, const int height) {
	return generate(nullptr, {
		.target = TextureTarget::Texture2D,
		.internalFormat = InternalFormat::Depth24,
		.format = BaseFormat::Depth,
		.parameters = {
			.minFilter = TextureFilter::Nearest,
			.magFilter = TextureFilter::Nearest,
			.wrapS = TextureWrap::ClampToEdge,
			.wrapT = TextureWrap::ClampToEdge,
		},
		.dataType = DataType::Float,
		.width = width,
		.height = height,
		.samples = 1,
		.layers = 1,
	});
}

Texture Texture::generateDepthAttachmentArray(const int width, const int height, const int layers) {
	return generate(nullptr, {
		.target = TextureTarget::Texture2DArray,
		.internalFormat = InternalFormat::Depth24,
		.format = BaseFormat::Depth,
		.parameters = {
			.minFilter = TextureFilter::Nearest,
			.magFilter = TextureFilter::Nearest,
			.wrapS = TextureWrap::ClampToEdge,
			.wrapT = TextureWrap::ClampToEdge,
		},
		.dataType = DataType::Float,
		.width = width,
		.height = height,
		.samples = 1,
		.layers = layers,
	});
}

Texture Texture::generateDepthAttachmentCubemapArray(const int width, const int height, const int layers) {
	return generate(nullptr, {
		.target = TextureTarget::TextureCubeMapArray,
		.internalFormat = InternalFormat::Depth24,
		.format = BaseFormat::Depth,
		.parameters = {
			.minFilter = TextureFilter::Nearest,
			.magFilter = TextureFilter::Nearest,
			.wrapS = TextureWrap::ClampToEdge,
			.wrapT = TextureWrap::ClampToEdge,
			.wrapR = TextureWrap::ClampToBorder,
		},
		.dataType = DataType::Float,
		.width = width,
		.height = height,
		.samples = 1,
		.layers = layers,
	});
}

void Texture::generateMipmaps(const TextureView handle) {
	glBindTexture(toUnderlying(handle.target), handle.id);
	glGenerateMipmap(toUnderlying(handle.target));
	glTexParameteri(toUnderlying(handle.target), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(toUnderlying(handle.target), 0);
}
