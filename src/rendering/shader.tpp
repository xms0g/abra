#pragma once

template<typename T>
void Shader::setValue(const std::string_view name, const T& value) {
	int32_t location = getUniformLocation(name);

	if constexpr (std::is_same_v<T, bool>)
		Uniform::setBool(location, value);
	else if constexpr (std::is_same_v<T, int32_t>)
		Uniform::setInt(location, value);
	else if constexpr (std::is_same_v<T, uint32_t>)
		Uniform::setUint(location, value);
	else if constexpr (std::is_same_v<T, float>)
		Uniform::setFloat(location, value);
	else if constexpr (std::is_same_v<T, glm::vec2>)
		Uniform::setVec2(location, value);
	else if constexpr (std::is_same_v<T, glm::vec3>)
		Uniform::setVec3(location, value);
	else if constexpr (std::is_same_v<T, glm::vec4>)
		Uniform::setVec4(location, value);
	else if constexpr (std::is_same_v<T, glm::mat2>)
		Uniform::setMat2(location, value);
	else if constexpr (std::is_same_v<T, glm::mat3>)
		Uniform::setMat3(location, value);
	else if constexpr (std::is_same_v<T, glm::mat4>)
		Uniform::setMat4(location, value);
	else
		static_assert(false, "Unsupported type");
}

template<typename T>
void Shader::setValue(const std::string_view name, const T* value, uint32_t count) {
	int32_t location = getUniformLocation(name);

	if constexpr (std::is_same_v<T, float>)
		Uniform::setFloatArray(location, value, count);
	else if constexpr (std::is_same_v<T, glm::mat4>)
		Uniform::setMat4Array(location, value, count);
}
