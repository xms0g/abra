#pragma once
#include <string>
#include <unordered_set>
#include "glad/glad.h"
#include "glm/glm.hpp"

enum class ShaderStageType: uint32_t {
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER,
	Geometry = GL_GEOMETRY_SHADER,
	TessControl = GL_TESS_CONTROL_SHADER,
	TessEvaluation = GL_TESS_EVALUATION_SHADER
};

struct ShaderStage {
	uint32_t handle{0};

	ShaderStageType type{};

	ShaderStage() = default;

	ShaderStage(const std::string& fn, ShaderStageType type);

	ShaderStage(const ShaderStage& other) = delete;

	ShaderStage& operator=(const ShaderStage& other) = delete;

	ShaderStage(ShaderStage&& other) noexcept;

	ShaderStage& operator=(ShaderStage&& other) noexcept;

	~ShaderStage();

	void checkCompileErrors(const std::string& fn) const;

	static std::string preprocess(const std::string& source);

	static std::string preprocess(const std::string& source, std::unordered_set<std::string>& includedFiles);
};

class Shader {
public:
	Shader();

	~Shader();

	Shader(const Shader&) = delete;

	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;

	Shader& operator=(Shader&& other) noexcept;

	[[nodiscard]]
	uint32_t id() const { return mID; }

	// use/activate the shader
	void bind() const;

	void attachStage(const ShaderStage& stage) const;

	void link() const;

	template<typename T>
	void setValue(const std::string& name, const T& value) const;

	template<typename T>
	void setValue(const std::string& name, const T* value, uint32_t count);

	// utility uniform functions
	void setBool(const std::string& name, bool value) const;

	void setInt(const std::string& name, int32_t value) const;

	void setUint(const std::string& name, uint32_t value) const;

	void setFloat(const std::string& name, float value) const;

	void setFloatArray(const std::string& name, const float* values, uint32_t count) const;

	void setVec2(const std::string& name, const glm::vec2& value) const;

	void setVec2(const std::string& name, float x, float y) const;

	void setVec3(const std::string& name, const glm::vec3& value) const;

	void setVec3(const std::string& name, float x, float y, float z) const;

	void setVec4(const std::string& name, const glm::vec4& value) const;

	void setVec4(const std::string& name, float x, float y, float z, float w) const;

	void setMat2(const std::string& name, const glm::mat2& mat) const;

	void setMat3(const std::string& name, const glm::mat3& mat) const;

	void setMat4(const std::string& name, const glm::mat4& mat) const;

	void setMat4Array(const std::string& name, const glm::mat4* matrices, size_t count) const;

	void checkLinkErrors() const;

private:
	// the program ID
	uint32_t mID{};
};

template<typename T>
void Shader::setValue(const std::string& name, const T& value) const {
	if constexpr (std::is_same_v<T, bool>)
		setBool(name, value);
	else if constexpr (std::is_same_v<T, int32_t>)
		setInt(name, value);
	else if constexpr (std::is_same_v<T, uint32_t>)
		setUint(name, value);
	else if constexpr (std::is_same_v<T, float>)
		setFloat(name, value);
	else if constexpr (std::is_same_v<T, glm::vec2>)
		setVec2(name, value);
	else if constexpr (std::is_same_v<T, glm::vec3>)
		setVec3(name, value);
	else if constexpr (std::is_same_v<T, glm::vec4>)
		setVec4(name, value);
	else if constexpr (std::is_same_v<T, glm::mat2>)
		setMat2(name, value);
	else if constexpr (std::is_same_v<T, glm::mat3>)
		setMat3(name, value);
	else if constexpr (std::is_same_v<T, glm::mat4>)
		setMat4(name, value);
	else
		static_assert(false, "Unsupported type");
}

template<typename T>
void Shader::setValue(const std::string& name, const T* value, uint32_t count) {
	if constexpr (std::is_same_v<T, float>)
		setFloatArray(name, value, count);
	else if constexpr (std::is_same_v<T, glm::mat4>)
		setMat4Array(name, value, count);
}
