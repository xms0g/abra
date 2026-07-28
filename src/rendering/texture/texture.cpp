#include "texture.h"
#include <iostream>
#include <utility>
#include "glad/glad.h"
#include "image/stb_image.h"
#include "../material/material.hpp"

Texture::Texture(const uint32_t id, const uint32_t type, const TextureTarget target, std::string path)
	: id(id),
	  type(type),
	  target(target),
	  path(std::move(path)) {
}

Texture::~Texture() {
	if (id != 0)
		glDeleteTextures(1, &id);
}

Texture::Texture(Texture&& other) noexcept
	: id(std::exchange(other.id, 0)),
	  type(std::exchange(other.type, 0)),
	  target(std::exchange(other.target, {})),
	  path(std::move(other.path)) {
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this == &other)
		return *this;
	id = std::exchange(other.id, 0);
	type = std::exchange(other.type, 0);
	target = std::exchange(other.target, {});
	path = std::move(other.path);
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

uint32_t Texture::load(const std::string& path, const uint32_t flags, const bool isSRGB) {
	uint32_t textureID;

	int32_t width, height, channel;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channel, 4);

	if (!data) {
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		exit(1);
	}

	const int32_t internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, flags & BLEND ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, flags & BLEND ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void Texture::info(const std::string& path, int32_t& width, int32_t& height) {
	int32_t channel;
	stbi_info(path.c_str(), &width, &height, &channel);
}

uint32_t Texture::loadCubemap(const std::vector<std::string>& faces) {
	uint32_t textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int32_t width, height, depth;
	for (uint32_t i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &depth, 0);

		if (!data) {
			std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			glDeleteTextures(1, &textureID);
			return 0;
		}

		const GLenum format = depth == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

Texture Texture::loadHDR(const std::string& path) {
	uint32_t texID;
	int32_t width, height, channel;

	stbi_set_flip_vertically_on_load(true);

	float* data = stbi_loadf(path.c_str(), &width, &height, &channel, 0);
	if (!data) {
		std::cerr << "HDR texture failed to load at path: " << path << std::endl;
		return {};
	}

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);

	stbi_set_flip_vertically_on_load(false);

	return {texID, 0, TextureTarget::Texture2D, ""};
}
