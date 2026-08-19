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

void Texture::upload(const uint32_t face,
					 const void* data,
					 const int32_t width,
					 const int32_t height,
					 BaseFormat format,
					 InternalFormat internalFormat,
					 DataType dataType) const {
	switch (target) {
		case TextureTarget::Texture2D: {
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(
				toUnderlying(target),
				0,
				toUnderlying(internalFormat),
				width,
				height,
				0,
				toUnderlying(format),
				toUnderlying(dataType),
				data);
			break;
		}
		case TextureTarget::TextureCubeMap: {
			assert(face < 6);
			glBindTexture(GL_TEXTURE_CUBE_MAP, id);

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				0,
				toUnderlying(internalFormat),
				width,
				height,
				0,
				toUnderlying(format),
				toUnderlying(dataType),
				data
			);
			break;
		}
	}
}

void Texture::info(const std::string_view path, int32_t& width, int32_t& height) {
	int32_t channel;
	stbi_info(path.data(), &width, &height, &channel);
}

void Texture::configureParameters(const TextureConfig& config) {
	if (config.target == TextureTarget::Texture2DMultisample) {
		return;
	}

	glTexParameteri(toUnderlying(config.target), GL_TEXTURE_WRAP_S, toUnderlying(config.parameters.wrapS));
	glTexParameteri(toUnderlying(config.target), GL_TEXTURE_WRAP_T, toUnderlying(config.parameters.wrapT));

	if (config.target == TextureTarget::TextureCubeMap || config.target == TextureTarget::TextureCubeMapArray) {
		glTexParameteri(toUnderlying(config.target), GL_TEXTURE_WRAP_R, toUnderlying(config.parameters.wrapR));
	}

	glTexParameteri(toUnderlying(config.target), GL_TEXTURE_MIN_FILTER, toUnderlying(config.parameters.minFilter));
	glTexParameteri(toUnderlying(config.target), GL_TEXTURE_MAG_FILTER, toUnderlying(config.parameters.magFilter));

	if (config.parameters.wrapS == TextureWrap::ClampToBorder ||
		config.parameters.wrapR == TextureWrap::ClampToBorder ||
		config.parameters.wrapT == TextureWrap::ClampToBorder) {
		constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(toUnderlying(config.target), GL_TEXTURE_BORDER_COLOR, borderColor);
	}
}

Texture Texture::load(const std::span<const std::string> paths, const TextureConfig& config) {
	int32_t width, height, channel;

	switch (config.target) {
		case TextureTarget::Texture2D: {
			TextureConfig cfg = config;
			void* data = nullptr;
			const char* path = paths[0].c_str();

			if (cfg.isHDR) {
				stbi_set_flip_vertically_on_load(true);

				data = stbi_loadf(path, &width, &height, &channel, 0);
				if (!data) {
					throw std::runtime_error(std::format("HDR texture failed to load at path: {}", path));
				}
			} else {
				data = stbi_load(path, &width, &height, &channel, 4);
				if (!data) {
					throw std::runtime_error(std::format("Texture failed to load at path: {}", path));
				}
			}

			cfg.width = width;
			cfg.height = height;
			auto texture = generate(cfg);

			texture.upload(0, data, width, height, config.format, config.internalFormat, config.dataType);

			stbi_image_free(data);
			stbi_set_flip_vertically_on_load(false);
			return texture;
		}
		case TextureTarget::TextureCubeMap: {
			TextureConfig cfg = config;

			if (paths.size() != 6) {
				throw std::runtime_error("Cubemap texture must have 6 paths");
			}

			info(paths[0], width, height);
			cfg.width = width;
			cfg.height = height;

			auto texture = generate(cfg);

			for (uint32_t i = 0; i < 6; ++i) {
				unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channel, 0);

				if (!data) {
					throw std::runtime_error(std::format("Cubemap texture failed to load at path: {}", paths[i]));
				}

				texture.upload(i, data, width, height, config.format, config.internalFormat, config.dataType);

				stbi_image_free(data);
			}

			return texture;
		}
		default: break;
	}

	return {};
}

Texture Texture::generate(const TextureConfig& config) {
	const auto target = toUnderlying(config.target);
	const auto format = toUnderlying(config.format);
	const auto internalFormat = toUnderlying(config.internalFormat);
	const auto dataType = toUnderlying(config.dataType);

	uint32_t textureID;
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
				nullptr);

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
				nullptr);

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
					nullptr);
			}

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
				nullptr);

			break;
		}
	}

	configureParameters(config);

	glBindTexture(target, 0);

	return {textureID, 0, config.target, ""};
}

Texture Texture::generateColorAttachment(const int32_t width, const int32_t height) {
	return generate({
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

Texture Texture::generateColorAttachmentRed(const int32_t width, const int32_t height) {
	return generate({
		.target = TextureTarget::Texture2D,
		.internalFormat = InternalFormat::Red8,
		.format = BaseFormat::Red,
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

Texture Texture::generateColorAttachmentMultisampled(const int32_t width, const int32_t height, const int32_t samples) {
	return generate({
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

Texture Texture::generateColorAttachmentFP(const int32_t width, const int32_t height) {
	return generate({
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

Texture Texture::generateColorAttachmentFPMultisampled(const int32_t width, const int32_t height, int32_t samples) {
	return generate({
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

Texture Texture::generateColorAttachmentCubemap(const int32_t width, const int32_t height) {
	return generate({
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

Texture Texture::generateDepthAttachment(const int32_t width, const int32_t height) {
	return generate({
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

Texture Texture::generateDepthAttachmentArray(const int32_t width, const int32_t height, const int32_t layers) {
	return generate({
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

Texture Texture::generateDepthAttachmentCubemapArray(const int32_t width, const int32_t height, const int32_t layers) {
	return generate({
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
