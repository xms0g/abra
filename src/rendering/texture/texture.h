#pragma once
#include <vector>
#include <string>

enum TextureType {
	DIFFUSE = 1,
	SPECULAR = 2,
	HEIGHT = 5,
	NORMAL = 6
};

struct Texture {
    uint32_t id;
	uint32_t type;
    std::string name;
    std::string path;
};

namespace texture {
uint32_t load(const char* path);

uint32_t loadCubemap(const std::vector<std::string>& faces);
}
