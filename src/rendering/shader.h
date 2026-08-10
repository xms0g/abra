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

struct PipelineShaderStage;

struct ShaderStage {
	uint32_t handle{0};

	ShaderStageType type{};

	ShaderStage() = default;

	explicit ShaderStage(const PipelineShaderStage& info);

	ShaderStage(const ShaderStage& other) = delete;

	ShaderStage& operator=(const ShaderStage& other) = delete;

	ShaderStage(ShaderStage&& other) noexcept;

	ShaderStage& operator=(ShaderStage&& other) noexcept;

	~ShaderStage();

	void checkCompileErrors(std::string_view code) const;

	static void preprocess(std::string_view source, std::string& output);

	static void preprocess(std::string_view source, std::string& output, std::unordered_set<std::string>& includedFiles);
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
	void setValue(std::string_view name, const T& value) const;

	template<typename T>
	void setValue(std::string_view name, const T* value, uint32_t count);

private:
	void checkLinkErrors() const;

	uint32_t mID{};
};

namespace Uniform {
void setBool(uint32_t id, std::string_view name, bool value);

void setInt(uint32_t id, std::string_view name, int32_t value);

void setUint(uint32_t id, std::string_view name, uint32_t value);

void setFloat(uint32_t id, std::string_view name, float value);

void setFloatArray(uint32_t id, std::string_view name, const float* values, uint32_t count);

void setVec2(uint32_t id, std::string_view name, const glm::vec2& value);

void setVec2(uint32_t id, std::string_view name, float x, float y);

void setVec3(uint32_t id, std::string_view name, const glm::vec3& value);

void setVec3(uint32_t id, std::string_view name, float x, float y, float z);

void setVec4(uint32_t id, std::string_view name, const glm::vec4& value);

void setVec4(uint32_t id, std::string_view name, float x, float y, float z, float w);

void setMat2(uint32_t id, std::string_view name, const glm::mat2& mat);

void setMat3(uint32_t id, std::string_view name, const glm::mat3& mat);

void setMat4(uint32_t id, std::string_view name, const glm::mat4& mat);

void setMat4Array(uint32_t id, std::string_view name, const glm::mat4* matrices, size_t count);
}

template<typename T>
void Shader::setValue(std::string_view name, const T& value) const {
	if constexpr (std::is_same_v<T, bool>)
		Uniform::setBool(mID, name, value);
	else if constexpr (std::is_same_v<T, int32_t>)
		Uniform::setInt(mID, name, value);
	else if constexpr (std::is_same_v<T, uint32_t>)
		Uniform::setUint(mID, name, value);
	else if constexpr (std::is_same_v<T, float>)
		Uniform::setFloat(mID, name, value);
	else if constexpr (std::is_same_v<T, glm::vec2>)
		Uniform::setVec2(mID, name, value);
	else if constexpr (std::is_same_v<T, glm::vec3>)
		Uniform::setVec3(mID, name, value);
	else if constexpr (std::is_same_v<T, glm::vec4>)
		Uniform::setVec4(mID, name, value);
	else if constexpr (std::is_same_v<T, glm::mat2>)
		Uniform::setMat2(mID, name, value);
	else if constexpr (std::is_same_v<T, glm::mat3>)
		Uniform::setMat3(mID, name, value);
	else if constexpr (std::is_same_v<T, glm::mat4>)
		Uniform::setMat4(mID, name, value);
	else
		static_assert(false, "Unsupported type");
}

template<typename T>
void Shader::setValue(std::string_view name, const T* value, uint32_t count) {
	if constexpr (std::is_same_v<T, float>)
		Uniform::setFloatArray(mID, name, value, count);
	else if constexpr (std::is_same_v<T, glm::mat4>)
		Uniform::setMat4Array(mID, name, value, count);
}

namespace ShaderLoader {
std::string load(std::string_view file);
}
