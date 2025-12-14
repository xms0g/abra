#include "texture.h"
#include <iostream>
#include "glad/glad.h"
#include "image/stb_image.h"
#include "../material/material.hpp"

uint32_t texture::load(const char* path) {
    uint32_t textureID;

	int width, height, channel;
    unsigned char* data = stbi_load(path, &width, &height, &channel, 0);

    if (!data) {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        exit(1);
    }

    GLenum format{0};
    if (channel == 1) {
    	format = GL_RED;
    } else if (channel == 3) {
    	format = GL_RGB;
    } else if (channel == 4) {
    	format = GL_RGBA;
    }

	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE: GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE: GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (channel == 1) {
		constexpr GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

uint32_t texture::loadCubemap(const std::vector<std::string>& faces) {
    uint32_t textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, depth;
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