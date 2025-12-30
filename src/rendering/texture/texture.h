#pragma once
#include <vector>
#include <string>

#include "glm/vec3.hpp"

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
uint32_t generate(uint32_t width, uint32_t height, const glm::vec3* data);

uint32_t load(const char* path, uint32_t flag);

uint32_t loadCubemap(const std::vector<std::string>& faces);
}
