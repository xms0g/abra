#include "shader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "enumUtils.hpp"
#include "../io/filesystem.hpp"
#include "../config/configManager.h"

GLu(ShaderStageType)

ShaderStage::ShaderStage(const std::string& fn, const ShaderStageType type) : type(type) {
	const auto processedSource = preprocess(
		fs::readFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + fn));

	handle = glCreateShader(toGLu(type));

	const char* ptr = processedSource.c_str();

	glShaderSource(handle, 1, &ptr, nullptr);
	glCompileShader(handle);

	checkCompileErrors(fn);
}

ShaderStage::ShaderStage(ShaderStage&& other) noexcept
	: handle(std::exchange(other.handle, 0)),
	  type(std::exchange(other.type, {})) {}

ShaderStage& ShaderStage::operator=(ShaderStage&& other) noexcept {
	if (this == &other)
		return *this;

	if (handle)
		glDeleteShader(handle);

	handle = std::exchange(other.handle, 0);
	type = std::exchange(other.type, {});
	return *this;
}

ShaderStage::~ShaderStage() {
	if (handle)
		glDeleteShader(handle);
}

void ShaderStage::checkCompileErrors(const std::string& fn) const {
	int32_t success;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

	if (!success) {
		std::string infoLog;
		int32_t maxLength = 0;

		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetShaderInfoLog(handle, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteShader(handle);

		throw std::runtime_error(std::format("Compilation Error in: {} \n {}", fn, infoLog));
	}
}

std::string ShaderStage::preprocess(const std::string& source) {
	std::unordered_set<std::string> includedFiles{};
	return preprocess(source, includedFiles);
}

std::string ShaderStage::preprocess(const std::string& source, std::unordered_set<std::string>& includedFiles) {
	if (source.empty()) return "";

	std::stringstream result;
	std::istringstream stream(source);
	std::string line;

	while (std::getline(stream, line)) {
		if (line.find("#include") == 0) {
			// Parse the include (e.g., #include "lighting.glsl")
			size_t start = line.find('"');
			size_t end = line.rfind('"');

			if (start != std::string::npos && end != std::string::npos && start != end) {
				std::string includeFile = line.substr(start + 1, end - start - 1);

				// Prevent cyclic includes
				if (includedFiles.contains(includeFile)) {
					continue;
				}
				includedFiles.insert(includeFile);
				// Load included file
				result << preprocess(
					fs::readFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + includeFile),
					includedFiles) << "\n";
			}
		} else {
			result << line << "\n";
		}
	}
	return result.str();
}

Shader::Shader() {
	mID = glCreateProgram();
}

Shader::~Shader() {
	if (mID)
		glDeleteProgram(mID);
}

Shader::Shader(Shader&& other) noexcept {
	mID = std::exchange(other.mID, 0);
}

Shader& Shader::operator=(Shader&& other) noexcept {
	if (this == &other)
		return *this;

	if (mID)
		glDeleteProgram(mID);

	mID = std::exchange(other.mID, 0);
	return *this;
}

void Shader::bind() const {
	glUseProgram(mID);
}

void Shader::attachStage(const ShaderStage& stage) const {
	glAttachShader(mID, stage.handle);
}

void Shader::link() const {
	glLinkProgram(mID);
	checkLinkErrors();
}

void Shader::checkLinkErrors() const {
	int32_t success;
	glGetProgramiv(mID, GL_LINK_STATUS, &success);

	if (!success) {
		std::string infoLog;
		int32_t maxLength = 0;

		glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetProgramInfoLog(mID, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteProgram(mID);

		throw std::runtime_error(std::string("Linking error:\n") + infoLog);
	}
}

void Uniform::setBool(const uint32_t id, const std::string& name, const bool value) {
	glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<int>(value));
}

void Uniform::setInt(const uint32_t id, const std::string& name, const int32_t value) {
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Uniform::setUint(const uint32_t id, const std::string& name, const uint32_t value) {
	glUniform1ui(glGetUniformLocation(id, name.c_str()), value);
}

void Uniform::setFloat(const uint32_t id, const std::string& name, const float value) {
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Uniform::setFloatArray(const uint32_t id, const std::string& name, const float* values, const uint32_t count) {
	glUniform1fv(glGetUniformLocation(id, name.c_str()), count, values);
}

void Uniform::setVec2(const uint32_t id, const std::string& name, const glm::vec2& value) {
	glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Uniform::setVec2(const uint32_t id, const std::string& name, const float x, const float y) {
	glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Uniform::setVec3(const uint32_t id, const std::string& name, const glm::vec3& value) {
	glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Uniform::setVec3(const uint32_t id, const std::string& name, const float x, const float y, const float z) {
	glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Uniform::setVec4(const uint32_t id, const std::string& name, const glm::vec4& value) {
	glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Uniform::setVec4(const uint32_t id, const std::string& name, const float x, const float y, const float z, const float w) {
	glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Uniform::setMat2(const uint32_t id, const std::string& name, const glm::mat2& mat) {
	glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Uniform::setMat3(const uint32_t id, const std::string& name, const glm::mat3& mat) {
	glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Uniform::setMat4(const uint32_t id, const std::string& name, const glm::mat4& mat) {
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Uniform::setMat4Array(const uint32_t id, const std::string& name, const glm::mat4* matrices, const size_t count) {
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), count, GL_FALSE, glm::value_ptr(matrices[0]));
}
