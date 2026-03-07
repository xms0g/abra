#include "shader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "glad/glad.h"
#include "../io/filesystem.hpp"
#include "../config/config.hpp"

Shader::Shader(const char* vs, const char* fs, const char* gs) {
	std::unordered_set<std::string> includedFiles{};

	try {
		const std::string vertexCode = preprocess(loadFile(vs), vs, includedFiles);
		includedFiles.clear();
		const std::string fragmentCode = preprocess(loadFile(fs), fs, includedFiles);
		includedFiles.clear();
		const std::string geometryCode = gs ? preprocess(loadFile(gs), gs, includedFiles) : "";
		includedFiles.clear();

		const uint32_t vertex = compileShader(vertexCode, vs, GL_VERTEX_SHADER);
		const uint32_t fragment = compileShader(fragmentCode, fs, GL_FRAGMENT_SHADER);
		uint32_t geometry = 0;

		if (gs) {
			geometry = compileShader(geometryCode, gs, GL_GEOMETRY_SHADER);
		}

		linkShader(vertex, fragment, geometry);
	} catch (std::runtime_error& e) {
		throw std::runtime_error(std::string("Shader ") + e.what());
	}
}

Shader::~Shader() {
	if (mID)
		glDeleteProgram(mID);
}

Shader::Shader(Shader&& other) noexcept {
	mID = other.mID;
	other.mID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
	if (this != &other) {
		glDeleteProgram(mID);
		mID = other.mID;
		other.mID = 0;
	}
	return *this;
}

void Shader::activate() const {
	glUseProgram(mID);
}

void Shader::setBool(const std::string& name, const bool value) const {
	glUniform1i(glGetUniformLocation(mID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, const int value) const {
	glUniform1i(glGetUniformLocation(mID, name.c_str()), value);
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

std::string Shader::loadFile(const char* fn) {
	std::ifstream file(fs::path(SHADER_DIR + fn));
	if (!file.is_open()) {
		throw std::runtime_error(std::string("Failed to open shader file: ") + fn);
	}
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

std::string Shader::preprocess(const std::string& source,
                               const char* fileName,
                               std::unordered_set<std::string>& includedFiles) {
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
				result << preprocess(loadFile(includeFile.c_str()), fileName, includedFiles) << "\n";
			}
		} else {
			result << line << "\n";
		}
	}
	return result.str();
}

uint32_t Shader::compileShader(const std::string& source, const char* fn, const uint32_t type) {
	const uint32_t shader = glCreateShader(type);
	const char* code = source.c_str();

	glShaderSource(shader, 1, &code, nullptr);
	glCompileShader(shader);

	// Check compile errors
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		std::string infoLog;
		GLint maxLength = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetShaderInfoLog(shader, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteShader(shader);

		throw std::runtime_error(std::string("Compilation Error in ") + fn + "\n" + infoLog);
	}

	return shader;
}

uint32_t Shader::linkShader(const uint32_t vertex, const uint32_t fragment, const uint32_t geometry) {
	mID = glCreateProgram();

	glAttachShader(mID, vertex);
	glAttachShader(mID, fragment);

	if (geometry != 0)
		glAttachShader(mID, geometry);

	glLinkProgram(mID);

	// Check linking errors
	GLint success;
	glGetProgramiv(mID, GL_LINK_STATUS, &success);

	if (!success) {
		std::string infoLog;
		GLint maxLength = 0;

		glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetProgramInfoLog(mID, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteProgram(mID);

		throw std::runtime_error(std::string("Linking error:\n") + infoLog);
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	if (geometry)
		glDeleteShader(geometry);

	return mID;
}
