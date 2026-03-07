#pragma once
#include <vector>
#include "glm/glm.hpp"

namespace math::random {
std::vector<glm::vec4> generateKernel(int sampleCount);

std::vector<float> generateNoise(int sampleCount);
}
