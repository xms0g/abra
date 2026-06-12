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

struct Texture {
    uint32_t id;
	uint32_t type;
    std::string path;

	void bind(uint32_t slot) const;
};

struct TextureBinding {
	std::string name;
	int32_t slot;
};

namespace texture {
Texture generate(int32_t width, int32_t height, const float* data);

uint32_t load(const char* path, uint32_t flags, bool isSRGB);

uint32_t loadCubemap(const std::vector<std::string>& faces);

Texture loadHDR(const char* path);
}
