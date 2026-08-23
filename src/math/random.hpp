#pragma once
#include <vector>
#include "glm/glm.hpp"

namespace math::random {
std::vector<glm::vec4> generateKernel(uint32_t sampleCount);

std::vector<float> generateNoise(uint32_t sampleCount);
}
