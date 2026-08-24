#pragma once
#include <string>
#include <unordered_map>
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

class ShaderStage {
public:
	ShaderStage() = default;

	explicit ShaderStage(const PipelineShaderStage& info);

	ShaderStage(const ShaderStage& other) = delete;

	ShaderStage& operator=(const ShaderStage& other) = delete;

	ShaderStage(ShaderStage&& other) noexcept;

	ShaderStage& operator=(ShaderStage&& other) noexcept;

	~ShaderStage();

	[[nodiscard]]
	uint32_t handle() const;

private:
	void checkCompileErrors(std::string_view code) const;

	static void preprocess(std::string_view source, std::string& output);

	static void preprocess(std::string_view source,
	                       std::string& output,
	                       std::unordered_set<std::string>& includedFiles);

	uint32_t mHandle{0};
	ShaderStageType type{};
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
	uint32_t handle() const { return mHandle; }

	// use/activate the shader
	void bind() const;

	void attachStage(const ShaderStage& stage) const;

	void link() const;

	template<typename T>
	void setValue(std::string_view name, const T& value);

	template<typename T>
	void setValue(std::string_view name, const T* value, uint32_t count);

private:
	void checkLinkErrors() const;

	[[nodiscard]]
	int32_t getUniformLocation(std::string_view name);

	struct StringHash {
		using is_transparent = void;

		size_t operator()(const std::string_view value) const noexcept {
			return std::hash<std::string_view>{}(value);
		}
	};

	struct StringEqual {
		using is_transparent = void;

		bool operator()(const std::string_view lhs, const std::string_view rhs) const noexcept {
			return lhs == rhs;
		}
	};

	uint32_t mHandle{};
	std::unordered_map<std::string, int32_t, StringHash, StringEqual> mUniformLocations;
};

namespace Uniform {
void setBool(int32_t location, bool value);

void setInt(int32_t location, int32_t value);

void setUint(int32_t location, uint32_t value);

void setFloat(int32_t location, float value);

void setFloatArray(int32_t location, const float* values, uint32_t count);

void setVec2(int32_t location, const glm::vec2& value);

void setVec2(int32_t location, float x, float y);

void setVec3(int32_t location, const glm::vec3& value);

void setVec3(int32_t location, float x, float y, float z);

void setVec4(int32_t location, const glm::vec4& value);

void setVec4(int32_t location, float x, float y, float z, float w);

void setMat2(int32_t location, const glm::mat2& mat);

void setMat3(int32_t location, const glm::mat3& mat);

void setMat4(int32_t location, const glm::mat4& mat);

void setMat4Array(int32_t location, const glm::mat4* matrices, size_t count);
}

namespace ShaderLoader {
std::string load(std::string_view file);
}

#include "shader.tpp"