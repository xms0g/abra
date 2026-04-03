#pragma once
#include <vector>
#include <string>

enum TextureType {
	ALBEDO = 1,
	SPECULAR = 2,
	EMISSION = 4,
	HEIGHT = 5,
	NORMAL = 6,
	METALLIC_ROUGHNESS = 18,
	AO = 17
};

struct Texture {
    uint32_t id;
	uint32_t type;
    std::string_view name;
    std::string path;
};

namespace texture {
uint32_t generate(int32_t width, int32_t height, const float* data);

uint32_t load(const char* path, uint32_t flag);

uint32_t loadCubemap(const std::vector<std::string>& faces);

uint32_t loadHDR(const char* path);
}
