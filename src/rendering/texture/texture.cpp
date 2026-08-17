#include "texture.h"
#include <iostream>
#include <utility>
#include "glad/glad.h"
#include "image/stb_image.h"
#include "../material/material.hpp"
#include "../glUtils.hpp"

Texture::Texture(const uint32_t id, const uint32_t type, const TextureTarget target, std::string path)
	: id(id),
	  target(target) {
}

Texture::~Texture() {
	if (id != 0)
		glDeleteTextures(1, &id);
}

Texture::Texture(Texture&& other) noexcept
	: id(std::exchange(other.id, 0)),
	  target(std::exchange(other.target, {})) {
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (id != 0)
			glDeleteTextures(1, &id);

		id = std::exchange(other.id, 0);
		target = std::exchange(other.target, {});
	}

	return *this;
}

Texture Texture::generate(const int32_t width, const int32_t height, const float* data) {
	uint32_t textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB16F,
		width,
		height,
		0,
		GL_RGB,
		GL_FLOAT,
		data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return {textureID, 0, TextureTarget::Texture2D, ""};
}

void Texture::generateMipmaps(const TextureView handle) {
	glBindTexture(toUnderlying(handle.target), handle.id);
	glGenerateMipmap(toUnderlying(handle.target));
	glTexParameteri(toUnderlying(handle.target), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(toUnderlying(handle.target), 0);
}

MaterialTexture::MaterialTexture(const uint32_t id, const uint32_t type, const TextureTarget target, std::string path)
	: id(id),
	  type(type),
	  target(target),
	  path(std::move(path)) {
}

MaterialTexture::~MaterialTexture() {
	if (id != 0)
		glDeleteTextures(1, &id);
}

MaterialTexture::MaterialTexture(MaterialTexture&& other) noexcept
	: id(std::exchange(other.id, 0)),
	  type(std::exchange(other.type, 0)),
	  target(std::exchange(other.target, {})),
	  path(std::move(other.path)) {
}

MaterialTexture& MaterialTexture::operator=(MaterialTexture&& other) noexcept {
	if (this != &other) {
		if (id != 0)
			glDeleteTextures(1, &id);

		id = std::exchange(other.id, 0);
		type = std::exchange(other.type, 0);
		target = std::exchange(other.target, {});
		path = std::move(other.path);
	}

	return *this;
}

void MaterialTexture::info(const std::string_view path, int32_t& width, int32_t& height) {
	int32_t channel;
	stbi_info(path.data(), &width, &height, &channel);
}

MaterialTexture MaterialTexture::load(const std::span<const std::string> paths, const TextureConfig& config) {
	uint32_t textureID;
	int32_t width, height, channel;

	const auto target = toUnderlying(config.target);
	const auto format = toUnderlying(config.format);
	const auto internalFormat = toUnderlying(config.internalFormat);
	const auto dataType = toUnderlying(config.dataType);

	glGenTextures(1, &textureID);
	glBindTexture(target, textureID);

	glTexParameteri(target, GL_TEXTURE_WRAP_S, toUnderlying(config.parameters.wrapS));
	glTexParameteri(target, GL_TEXTURE_WRAP_T, toUnderlying(config.parameters.wrapT));
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, toUnderlying(config.parameters.minFilter));
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, toUnderlying(config.parameters.magFilter));

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
			} else {
				data = stbi_load(path, &width, &height, &channel, 4);
				if (!data) {
					throw std::runtime_error(std::format("Texture failed to load at path: {}", path));
				}
			}

			glTexImage2D(
				target,
				0,
				internalFormat,
				width,
				height,
				0,
				format,
				dataType,
				data);

			stbi_image_free(data);
			glBindTexture(target, 0);

			stbi_set_flip_vertically_on_load(false);
			return {textureID, 0, TextureTarget::Texture2D, path};
		}
		case TextureTarget::TextureCubeMap: {
			for (uint32_t i = 0; i < paths.size(); i++) {
				unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channel, 0);

				if (!data) {
					glDeleteTextures(1, &textureID);
					throw std::runtime_error(std::format("Cubemap texture failed to load at path: {}", paths[i]));
				}

				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					internalFormat,
					width,
					height,
					0,
					format,
					dataType,
					data);

				stbi_image_free(data);
			}
			glTexParameteri(target, GL_TEXTURE_WRAP_R, toUnderlying(config.parameters.wrapR));
			glBindTexture(target, 0);
			return {textureID, 0, TextureTarget::TextureCubeMap, ""};
		}
	}
	return {};
}