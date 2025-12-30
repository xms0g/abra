#pragma once
#include <random>
#include <vector>
#include "glm/glm.hpp"
#include "lerp.hpp"

namespace math::random {
static inline std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
static inline std::default_random_engine generator;

inline std::vector<glm::vec3> generateKernel(const int sampleCount) {
	std::vector<glm::vec3> kernel;

	for (unsigned int i = 0; i < sampleCount; ++i) {
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
		                 randomFloats(generator) * 2.0 - 1.0,
		                 randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);

		float scale = static_cast<float>(i) / static_cast<float>(sampleCount);
		// scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		kernel.push_back(sample);
	}

	return kernel;
}

inline std::vector<float> generateNoise(const int sampleCount) {
	std::vector<float> noises;

	for (unsigned int i = 0; i < sampleCount; i++) {
		noises.push_back(randomFloats(generator) * 2.0f - 1.0f);
		noises.push_back(randomFloats(generator) * 2.0f - 1.0f);
		noises.push_back(0.0f);
	}

	return noises;
}
}
