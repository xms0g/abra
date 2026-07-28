#pragma once
#include <vector>
#include <string>
#include "glad/glad.h"
#include "../enumUtils.hpp"

enum TextureType {
	ALBEDO = 1,
	SPECULAR = 2,
	EMISSION = 4,
	HEIGHT = 5,
	NORMAL = 6,
	AO = 10,
	ROUGHNESS_METALLIC = 18
};

enum class TextureTarget : uint32_t {
	Texture2D = GL_TEXTURE_2D,
	Texture2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
	Texture2DArray = GL_TEXTURE_2D_ARRAY,
	TextureCubeMap = GL_TEXTURE_CUBE_MAP,
	TextureCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY
};

struct TextureBinding {
	const char* name;
	int32_t slot;
};

struct TextureHandle {
	uint32_t id{0};
	TextureTarget target{};
};

struct Texture {
	uint32_t id{};
	uint32_t type{};
	TextureTarget target{};
	std::string path;

	Texture() = default;

	Texture(uint32_t id, uint32_t type, TextureTarget target = TextureTarget::Texture2D, std::string path = "");

	~Texture();

	Texture(const Texture&) = delete;

	Texture& operator=(const Texture&) = delete;

	Texture(Texture&& other) noexcept;

	Texture& operator=(Texture&& other) noexcept;

	static Texture generate(int32_t width, int32_t height, const float* data);

	static uint32_t load(const std::string& path, uint32_t flags, bool isSRGB);

	static void info(const std::string& path, int32_t& width, int32_t& height);

	static uint32_t loadCubemap(const std::vector<std::string>& faces);

	static Texture loadHDR(const std::string& path);
};

GL(TextureTarget)
