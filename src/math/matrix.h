#pragma once
#include "glm/glm.hpp"

namespace math {
glm::mat4 modelMatrix(const glm::vec3& position,
                      const glm::vec3& rotation,
                      const glm::vec3& scale);

glm::mat3 normalMatrix(const glm::mat4& model);
}
