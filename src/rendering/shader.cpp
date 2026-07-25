#include "shader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "../io/filesystem.hpp"
#include "../config/configManager.h"

ShaderResource::ShaderResource(const char** code, const std::string& fn, const uint32_t type) {
	handle = glCreateShader(type);
	glShaderSource(handle, 1, code, nullptr);
	glCompileShader(handle);

	checkCompileErrors(fn);
}

ShaderResource::ShaderResource(ShaderResource&& other) noexcept {
	handle = other.handle;
	other.handle = 0;
}

ShaderResource& ShaderResource::operator=(ShaderResource&& other) noexcept {
	if (this == &other) return *this;

	handle = other.handle;
	other.handle = 0;
	return *this;
}

ShaderResource::~ShaderResource() {
	if (handle)
		glDeleteShader(handle);
}

void ShaderResource::attach(const uint32_t programID) const {
	if (handle)
		glAttachShader(programID, handle);
}

void ShaderResource::checkCompileErrors(const std::string& fn) const {
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

		throw std::runtime_error(std::string("Compilation Error in ") + fn + "\n" + infoLog);
	}
}

Shader::Shader(const std::string& vs, const std::string& fs, const std::string& gs, const std::string& tcs, const std::string& tes) {
	try {
		const std::string vertexSource = preprocess(fs::loadFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + vs));
		const std::string fragmentSource = preprocess(fs::loadFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + fs));
		const std::string geometrySource = preprocess(fs::loadFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + gs));
		const std::string tessControlSource = preprocess(fs::loadFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + tcs));
		const std::string tessEvalSource = preprocess(fs::loadFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + tes));

		auto vert = compileShader(vertexSource, vs, GL_VERTEX_SHADER);
		auto frag = compileShader(fragmentSource, fs, GL_FRAGMENT_SHADER);

		ShaderResource geo, tessControl, tessEval;
		if (!geometrySource.empty()) {
			geo = compileShader(geometrySource, gs, GL_GEOMETRY_SHADER);
		}

		if (!tessControlSource.empty()) {
			tessControl = compileShader(tessControlSource, tcs, GL_TESS_CONTROL_SHADER);
		}

		if (!tessEvalSource.empty()) {
			tessEval = compileShader(tessEvalSource, tes, GL_TESS_EVALUATION_SHADER);
		}

		linkShader(mID, vert, frag, geo, tessControl, tessEval);
	} catch (std::runtime_error& e) {
		throw std::runtime_error(std::string("Shader ") + e.what());
	}
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

void Shader::setBool(const char* name, const bool value) const {
	glUniform1i(glGetUniformLocation(mID, name), static_cast<int>(value));
}

void Shader::setInt(const char* name, const int32_t value) const {
	glUniform1i(glGetUniformLocation(mID, name), value);
}

void Shader::setUint(const char* name, const uint32_t value) const {
	glUniform1ui(glGetUniformLocation(mID, name), value);
}

void Shader::setFloat(const char* name, const float value) const {
	glUniform1f(glGetUniformLocation(mID, name), value);
}

void Shader::setFloatArray(const char* name, const float* values, const uint32_t count) const {
	glUniform1fv(glGetUniformLocation(mID, name), count, values);
}

void Shader::setVec2(const char* name, const glm::vec2& value) const {
	glUniform2fv(glGetUniformLocation(mID, name), 1, &value[0]);
}

void Shader::setVec2(const char* name, const float x, const float y) const {
	glUniform2f(glGetUniformLocation(mID, name), x, y);
}

void Shader::setVec3(const char* name, const glm::vec3& value) const {
	glUniform3fv(glGetUniformLocation(mID, name), 1, &value[0]);
}

void Shader::setVec3(const char* name, const float x, const float y, const float z) const {
	glUniform3f(glGetUniformLocation(mID, name), x, y, z);
}

void Shader::setVec4(const char* name, const glm::vec4& value) const {
	glUniform4fv(glGetUniformLocation(mID, name), 1, &value[0]);
}

void Shader::setVec4(const char* name, const float x, const float y, const float z, const float w) const {
	glUniform4f(glGetUniformLocation(mID, name), x, y, z, w);
}

void Shader::setMat2(const char* name, const glm::mat2& mat) const {
	glUniformMatrix2fv(glGetUniformLocation(mID, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const char* name, const glm::mat3& mat) const {
	glUniformMatrix3fv(glGetUniformLocation(mID, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const char* name, const glm::mat4& mat) const {
	glUniformMatrix4fv(glGetUniformLocation(mID, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4Array(const char* name, const glm::mat4* matrices, const size_t count) const {
	glUniformMatrix4fv(glGetUniformLocation(mID, name), count, GL_FALSE, glm::value_ptr(matrices[0]));
}

std::string Shader::preprocess(const std::string& source) {
	std::unordered_set<std::string> includedFiles{};
	return preprocess(source, includedFiles);
}

std::string Shader::preprocess(const std::string& source, std::unordered_set<std::string>& includedFiles) {
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
				result << preprocess(fs::loadFile(CONFIG_MANAGER_INSTANCE.get<std::string>("path.shader") + includeFile.c_str()), includedFiles) << "\n";
			}
		} else {
			result << line << "\n";
		}
	}
	return result.str();
}

ShaderResource Shader::compileShader(const std::string& source, const std::string& fn, const uint32_t type) {
	const char* code = source.c_str();

	ShaderResource resource{&code, fn, type};
	return std::move(resource);
}

void Shader::linkShader(
	uint32_t& programID,
	const ShaderResource& vert,
	const ShaderResource& frag,
	const ShaderResource& geo,
	const ShaderResource& tessControl,
	const ShaderResource& tessEval) {
	programID = glCreateProgram();

	vert.attach(programID);
	frag.attach(programID);
	geo.attach(programID);
	tessControl.attach(programID);
	tessEval.attach(programID);

	glLinkProgram(programID);

	checkLinkErrors(programID);
}

void Shader::checkLinkErrors(const uint32_t programID) {
	int32_t success;
	glGetProgramiv(programID, GL_LINK_STATUS, &success);

	if (!success) {
		std::string infoLog;
		int32_t maxLength = 0;

		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetProgramInfoLog(programID, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteProgram(programID);

		throw std::runtime_error(std::string("Linking error:\n") + infoLog);
	}
}