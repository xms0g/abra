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
	const auto code = fs::readFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + fn);
	const auto processedSource = preprocess(code);

	handle = glCreateShader(toGLu(type));

	const char* ptr = processedSource.c_str();

	glShaderSource(handle, 1, &ptr, nullptr);
	glCompileShader(handle);

	try {
		checkCompileErrors(fn);
	} catch (std::runtime_error& e) {
	}
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

void Shader::setBool(const std::string& name, const bool value) const {
	glUniform1i(glGetUniformLocation(mID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, const int32_t value) const {
	glUniform1i(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::setUint(const std::string& name, const uint32_t value) const {
	glUniform1ui(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, const float value) const {
	glUniform1f(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::setFloatArray(const std::string& name, const float* values, const uint32_t count) const {
	glUniform1fv(glGetUniformLocation(mID, name.c_str()), count, values);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
	glUniform2fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string& name, const float x, const float y) const {
	glUniform2f(glGetUniformLocation(mID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
	glUniform3fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const float x, const float y, const float z) const {
	glUniform3f(glGetUniformLocation(mID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
	glUniform4fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const float x, const float y, const float z, const float w) const {
	glUniform4f(glGetUniformLocation(mID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const {
	glUniformMatrix2fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
	glUniformMatrix3fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
	glUniformMatrix4fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4Array(const std::string& name, const glm::mat4* matrices, const size_t count) const {
	glUniformMatrix4fv(glGetUniformLocation(mID, name.c_str()), count, GL_FALSE, glm::value_ptr(matrices[0]));
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
