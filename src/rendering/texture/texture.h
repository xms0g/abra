#pragma once
#include <vector>
#include <string>
#include "glad/glad.h"

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

enum class BaseFormat : int32_t {
	Red = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGBA = GL_RGBA,
};

enum class InternalFormat : int32_t {
	Red = GL_R8,
	RG = GL_RG8,
	RGB = GL_RGB8,
	RGBA = GL_RGBA8,
	RedFloat = GL_R16F,
	RGFloat = GL_RG16F,
	RGBFloat = GL_RGB16F,
	RGBAFloat = GL_RGBA16F,
	Depth24 = GL_DEPTH_COMPONENT24,
	Depth32F = GL_DEPTH_COMPONENT32F,
};

struct TextureConfig {
	TextureTarget target{};
	InternalFormat internalFormat{};
	BaseFormat format{};
	int width{};
	int height{};
	int samples{1};
	int layers{1};
};

struct TextureView {
	uint32_t id{0};
	TextureTarget target{};

	bool operator==(const TextureView& other) const = default;
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

	static void generateMipmaps(TextureView handle);
};

struct MaterialTexture {
	uint32_t id{};
	uint32_t type{};
	TextureTarget target{};
	std::string path;

	MaterialTexture() = default;

	MaterialTexture(uint32_t id, uint32_t type, TextureTarget target = TextureTarget::Texture2D, std::string path = "");

	~MaterialTexture();

	MaterialTexture(const MaterialTexture&) = delete;

	MaterialTexture& operator=(const MaterialTexture&) = delete;

	MaterialTexture(MaterialTexture&& other) noexcept;

	MaterialTexture& operator=(MaterialTexture&& other) noexcept;

	static void info(std::string_view path, int32_t& width, int32_t& height);

	static uint32_t load(std::string_view path, uint32_t flags, bool isSRGBA);

	static MaterialTexture loadCubemap(const std::vector<std::string>& faces);

	static MaterialTexture loadHDR(std::string_view path);
};
