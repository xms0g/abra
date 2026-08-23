#pragma once
#include <vector>
#include <string>
#include <span>
#include "glad/glad.h"

enum class TextureType {
	Albedo,
	Specular,
	Emissive,
	Height,
	Normal,
	Ao,
	Roughness_Metallic
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

class GPUTexture;

struct MaterialTexture {
	std::string path;
	TextureType type{};
	std::shared_ptr<GPUTexture> texture;

	MaterialTexture() = default;

	MaterialTexture(std::string path, TextureType type, std::shared_ptr<GPUTexture> texture);

	MaterialTexture(const MaterialTexture& other) = delete;

	MaterialTexture& operator=(const MaterialTexture& other) = delete;

	MaterialTexture(MaterialTexture&& other) noexcept;

	MaterialTexture& operator=(MaterialTexture&& other) noexcept;

	static void info(std::string_view path, int32_t& width, int32_t& height);

	static MaterialTexture load(std::span<const std::string> paths, const TextureConfig& config);
};

class GPUTexture {
public:
	GPUTexture() = default;

	explicit GPUTexture(const TextureConfig& config);

	~GPUTexture();

	GPUTexture(const GPUTexture& other) = delete;

	GPUTexture& operator=(const GPUTexture& other) = delete;

	GPUTexture(GPUTexture&& other) noexcept;

	GPUTexture& operator=(GPUTexture&& other) noexcept;

	[[nodiscard]]
	uint32_t id() const;

	[[nodiscard]]
	TextureTarget target() const;

	void copyToMemory(const void* pixels, uint32_t face) const;

	void generateMipmaps() const;

	static void configureParameters(const TextureConfig& config);

	static std::shared_ptr<GPUTexture> generateColorAttachment(int32_t width, int32_t height);

	static std::shared_ptr<GPUTexture> generateColorAttachmentRed(int32_t width, int32_t height);

	static std::shared_ptr<GPUTexture> generateColorAttachmentMultisampled(int32_t width, int32_t height, int32_t samples);

	static std::shared_ptr<GPUTexture> generateColorAttachmentFP(int32_t width, int32_t height);

	static std::shared_ptr<GPUTexture> generateColorAttachmentFPMultisampled(int32_t width, int32_t height, int32_t samples);

	static std::shared_ptr<GPUTexture> generateColorAttachmentCubemap(int32_t width, int32_t height);

	static std::shared_ptr<GPUTexture> generateDepthAttachment(int32_t width, int32_t height);

	static std::shared_ptr<GPUTexture> generateDepthAttachmentArray(int32_t width, int32_t height, int32_t layers);

	static std::shared_ptr<GPUTexture> generateDepthAttachmentCubemapArray(int32_t width, int32_t height, int32_t layers);

private:
	uint32_t mID{};
	TextureTarget mTarget{};
	int32_t mWidth{};
	int32_t mHeight{};
	BaseFormat mFormat{};
	DataType mDataType{};
};
