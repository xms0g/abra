#include "matrix.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

glm::mat4 math::modelMatrix(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
	glm::mat4 model(1.0f);
	model = glm::translate(model, position);
	model *= glm::toMat4(glm::quat(glm::radians(rotation)));
	model *= glm::scale(glm::mat4(1.0f), scale);

	return model;
}

glm::mat3 math::normalMatrix(const glm::mat4& model) {
	return glm::transpose(glm::inverse(glm::mat3(model)));
}
