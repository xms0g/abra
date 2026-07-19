#pragma once
#include <vector>
#include <string>

enum TextureType {
	ALBEDO = 1,
	SPECULAR = 2,
	EMISSION = 4,
	HEIGHT = 5,
	NORMAL = 6,
	AO = 10,
	ROUGHNESS_METALLIC = 18
};

struct TextureBinding {
	const char* name;
	int32_t slot;
};

struct Texture {
    uint32_t id;
	uint32_t type;
    std::string path;

	Texture() = default;

	Texture(uint32_t id, uint32_t type, const std::string& path);

	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;

    Texture& operator=(Texture&& other) noexcept;

    void bind(uint32_t slot) const;

	static Texture generate(int32_t width, int32_t height, const float* data);

	static uint32_t load(const std::string& path, uint32_t flags, bool isSRGB);

	static void info(const std::string& path, int32_t& width, int32_t& height);

	static uint32_t loadCubemap(const std::vector<std::string>& faces);

	static Texture loadHDR(const std::string& path);
};
