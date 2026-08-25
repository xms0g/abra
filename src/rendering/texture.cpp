#include "texture.hpp"
#include <cassert>
#include <iostream>
#include <utility>
#include "glad/glad.h"
#include "image/stb_image.h"

MaterialTexture::MaterialTexture(std::string path, const TextureType type, std::shared_ptr<GPUTexture> texture)
	: path(std::move(path)),
	  type(type),
	  texture(std::move(texture)) {
}

void MaterialTexture::info(const std::string_view path, int32_t& width, int32_t& height) {
	int32_t channel;
	stbi_info(path.data(), &width, &height, &channel);
}

MaterialTexture MaterialTexture::load(const std::span<const std::string> paths, const TextureConfig& config) {
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
			auto gpuPtr = std::make_shared<GPUTexture>(cfg);

			gpuPtr->copyToMemory(data, 0);

			stbi_image_free(data);
			stbi_set_flip_vertically_on_load(false);
			return {path, TextureType::Albedo, std::move(gpuPtr)};
		}
		case TextureTarget::TextureCubeMap: {
			TextureConfig cfg = config;

			info(paths[0], width, height);
			cfg.width = width;
			cfg.height = height;

			auto gpuPtr = std::make_shared<GPUTexture>(cfg);

			for (uint32_t i = 0; i < 6; ++i) {
				unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channel, 0);

				if (!data) {
					throw std::runtime_error(std::format("Cubemap texture failed to load at path: {}", paths[i]));
				}

				gpuPtr->copyToMemory(data, i);

				stbi_image_free(data);
			}

			return {"", TextureType::Albedo, std::move(gpuPtr)};
		}
		default:
			break;
	}

	return {};
}

GPUTexture::GPUTexture(const TextureConfig& config) {
	mInfo.target = config.target;
	mInfo.width = config.width;
	mInfo.height = config.height;
	mInfo.format = config.format;
	mInfo.dataType = config.dataType;
	const auto target = std::to_underlying(config.target);
	const auto format = std::to_underlying(config.format);
	const auto internalFormat = std::to_underlying(config.internalFormat);
	const auto dataType = std::to_underlying(config.dataType);

	glGenTextures(1, &mID);
	glBindTexture(target, mID);

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
}

GPUTexture::~GPUTexture() {
	if (mID)
		glDeleteTextures(1, &mID);
}

GPUTexture::GPUTexture(GPUTexture&& other) noexcept
	: mID(std::exchange(other.mID, 0)),
	  mInfo(std::exchange(other.mInfo, {})) {
}

GPUTexture& GPUTexture::operator=(GPUTexture&& other) noexcept {
	if (this != &other) {
		if (mID)
			glDeleteTextures(1, &mID);

		mID = std::exchange(other.mID, 0);
		mInfo = std::exchange(other.mInfo, {});
	}

	return *this;
}

uint32_t GPUTexture::id() const {
	return mID;
}

TextureTarget GPUTexture::target() const {
	return mInfo.target;
}

void GPUTexture::copyToMemory(const void* pixels, const uint32_t face) const {
	switch (mInfo.target) {
		case TextureTarget::Texture2D: {
			glBindTexture(GL_TEXTURE_2D, mID);
			glTexSubImage2D(
				std::to_underlying(mInfo.target),
				0,
				0,
				0,
				mInfo.width,
				mInfo.height,
				std::to_underlying(mInfo.format),
				std::to_underlying(mInfo.dataType),
				pixels);
			break;
		}
		case TextureTarget::TextureCubeMap: {
			assert(face < 6);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mID);

			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				0,
				0,
				0,
				mInfo.width,
				mInfo.height,
				std::to_underlying(mInfo.format),
				std::to_underlying(mInfo.dataType),
				pixels
			);
			break;
		}
		default: break;
	}
}

void GPUTexture::generateMipmaps() const {
	glBindTexture(std::to_underlying(mInfo.target), mID);
	glGenerateMipmap(std::to_underlying(mInfo.target));
	glTexParameteri(std::to_underlying(mInfo.target), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(std::to_underlying(mInfo.target), 0);
}

void GPUTexture::configureParameters(const TextureConfig& config) {
	if (config.target == TextureTarget::Texture2DMultisample) {
		return;
	}

	glTexParameteri(std::to_underlying(config.target), GL_TEXTURE_WRAP_S, std::to_underlying(config.parameters.wrapS));
	glTexParameteri(std::to_underlying(config.target), GL_TEXTURE_WRAP_T, std::to_underlying(config.parameters.wrapT));

	if (config.target == TextureTarget::TextureCubeMap || config.target == TextureTarget::TextureCubeMapArray) {
		glTexParameteri(std::to_underlying(config.target), GL_TEXTURE_WRAP_R, std::to_underlying(config.parameters.wrapR));
	}

	glTexParameteri(std::to_underlying(config.target), GL_TEXTURE_MIN_FILTER, std::to_underlying(config.parameters.minFilter));
	glTexParameteri(std::to_underlying(config.target), GL_TEXTURE_MAG_FILTER, std::to_underlying(config.parameters.magFilter));

	if (config.parameters.wrapS == TextureWrap::ClampToBorder ||
	    config.parameters.wrapR == TextureWrap::ClampToBorder ||
	    config.parameters.wrapT == TextureWrap::ClampToBorder) {
		constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(std::to_underlying(config.target), GL_TEXTURE_BORDER_COLOR, borderColor);
	}
}

std::shared_ptr<GPUTexture> GPUTexture::generateColorAttachment(const int32_t width, const int32_t height) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateColorAttachmentRed(const int32_t width, const int32_t height) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateColorAttachmentMultisampled(const int32_t width, const int32_t height, const int32_t samples) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateColorAttachmentFP(const int32_t width, const int32_t height) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateColorAttachmentFPMultisampled(const int32_t width, const int32_t height, const int32_t samples) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateColorAttachmentCubemap(const int32_t width, const int32_t height) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateDepthAttachment(const int32_t width, const int32_t height) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateDepthAttachmentArray(const int32_t width, const int32_t height, const int32_t layers) {
	return std::make_shared<GPUTexture>(TextureConfig{
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

std::shared_ptr<GPUTexture> GPUTexture::generateDepthAttachmentCubemapArray(const int32_t width, const int32_t height, const int32_t layers) {
	return std::make_shared<GPUTexture>(TextureConfig{
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
