#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <span>
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
	Depth = GL_DEPTH_COMPONENT,
};

enum class InternalFormat : int32_t {
	Red8 = GL_R8,
	RG8 = GL_RG8,
	RGB8 = GL_RGB8,
	RGBA8 = GL_RGBA8,
	RedFloat = GL_R16F,
	RGFloat = GL_RG16F,
	RGBFloat = GL_RGB16F,
	RGBAFloat = GL_RGBA16F,
	SRGB8 = GL_SRGB8,
	SRGB8Alpha8 = GL_SRGB8_ALPHA8,
	Depth24 = GL_DEPTH_COMPONENT24,
	Depth32F = GL_DEPTH_COMPONENT32F,
};

enum class TextureFilter : int32_t {
	Nearest = GL_NEAREST,
	Linear = GL_LINEAR,
};

enum class TextureWrap : int32_t {
	Repeat = GL_REPEAT,
	MirroredRepeat = GL_MIRRORED_REPEAT,
	ClampToEdge = GL_CLAMP_TO_EDGE,
	ClampToBorder = GL_CLAMP_TO_BORDER,
};

enum class DataType : int32_t {
	Float = GL_FLOAT,
	UnsignedByte = GL_UNSIGNED_BYTE,
	UnsignedShort = GL_UNSIGNED_SHORT,
	UnsignedInt = GL_UNSIGNED_INT,
};

struct TextureParameters {
	TextureFilter minFilter = TextureFilter::Linear;
	TextureFilter magFilter = TextureFilter::Linear;

	TextureWrap wrapS = TextureWrap::ClampToEdge;
	TextureWrap wrapT = TextureWrap::ClampToEdge;
	TextureWrap wrapR = TextureWrap::ClampToEdge;
};

struct TextureConfig {
	TextureTarget target{};
	InternalFormat internalFormat{};
	BaseFormat format{};
	TextureParameters parameters{};
	DataType dataType{};
	bool isHDR{false};
	int32_t width{};
	int32_t height{};
	int32_t samples{1};
	int32_t layers{1};
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

	Texture(uint32_t id, uint32_t type, TextureTarget target, std::string path = "");

	~Texture();

	Texture(const Texture&) = delete;

	Texture& operator=(const Texture&) = delete;

	Texture(Texture&& other) noexcept;

	Texture& operator=(Texture&& other) noexcept;

	void upload(uint32_t face,
				const void* data,
				int32_t width,
				int32_t height,
				BaseFormat format,
				InternalFormat internalFormat,
				DataType dataType) const;

	static void info(std::string_view path, int32_t& width, int32_t& height);

	static void configureParameters(const TextureConfig& config);

	static Texture load(std::span<const std::string> paths, const TextureConfig& config);

	static Texture generate(const TextureConfig& config);

	static Texture generateColorAttachment(int32_t width, int32_t height);

	static Texture generateColorAttachmentRed(int32_t width, int32_t height);

	static Texture generateColorAttachmentMultisampled(int32_t width, int32_t height, int32_t samples);

	static Texture generateColorAttachmentFP(int32_t width, int32_t height);

	static Texture generateColorAttachmentFPMultisampled(int32_t width, int32_t height, int32_t samples);

	static Texture generateColorAttachmentCubemap(int32_t width, int32_t height);

	static Texture generateDepthAttachment(int32_t width, int32_t height);

	static Texture generateDepthAttachmentArray(int32_t width, int32_t height, int32_t layers);

	static Texture generateDepthAttachmentCubemapArray(int32_t width, int32_t height, int32_t layers);

	static void generateMipmaps(TextureView handle);
};
