#pragma once
#include <vector>
#include <string>

enum TextureType {
	ALBEDO = 1,
	SPECULAR = 2,
	EMISSION = 4,
	HEIGHT = 5,
	NORMAL = 6,
	METALNESS = 15,
	ROUGHNESS = 16,
	AO = 17
};

struct Texture {
    uint32_t id;
	uint32_t type;
    std::string_view name;
    std::string path;
};

namespace texture {
uint32_t generate(uint32_t width, uint32_t height, const float* data);

uint32_t generateCubemap(uint32_t size);

uint32_t load(const char* path, uint32_t flag);

uint32_t loadCubemap(const std::vector<std::string>& faces);

uint32_t loadHDR(const char* path);
}
