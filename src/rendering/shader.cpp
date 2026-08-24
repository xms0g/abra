#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "../io/filesystem.hpp"
#include "../config/configManager.hpp"
#include "../rendering/graphicsPipeline.hpp"

ShaderStage::ShaderStage(const PipelineShaderStage& info) : type(info.stage) {
	std::string processedSource;
	preprocess(info.code, processedSource);

	mHandle = glCreateShader(std::to_underlying(info.stage));

	const char* ptr = processedSource.data();

	glShaderSource(mHandle, 1, &ptr, nullptr);
	glCompileShader(mHandle);

	checkCompileErrors(info.code);
}

ShaderStage::ShaderStage(ShaderStage&& other) noexcept
	: mHandle(std::exchange(other.mHandle, 0)),
	  type(std::exchange(other.type, {})) {
}

ShaderStage& ShaderStage::operator=(ShaderStage&& other) noexcept {
	if (this != &other) {
		if (mHandle)
			glDeleteShader(mHandle);

		mHandle = std::exchange(other.mHandle, 0);
		type = std::exchange(other.type, {});
	}

	return *this;
}

ShaderStage::~ShaderStage() {
	if (mHandle)
		glDeleteShader(mHandle);
}

uint32_t ShaderStage::handle() const {
	return mHandle;
}

void ShaderStage::checkCompileErrors(const std::string_view code) const {
	int32_t success;
	glGetShaderiv(mHandle, GL_COMPILE_STATUS, &success);

	if (!success) {
		std::string infoLog;
		int32_t maxLength = 0;

		glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetShaderInfoLog(mHandle, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteShader(mHandle);

		throw std::runtime_error(std::format("Compilation Error in: {} \n {}", code, infoLog));
	}
}

void ShaderStage::preprocess(const std::string_view source, std::string& output) {
	std::unordered_set<std::string> includedFiles{};
	preprocess(source, output, includedFiles);
}

void ShaderStage::preprocess(const std::string_view source, std::string& output, std::unordered_set<std::string>& includedFiles) {
	if (source.empty()) return;

	static std::filesystem::path shaderRoot = CONFIG_MANAGER.get<std::string>("path.shader");

	std::istringstream stream(source.data());
	std::string line;

	while (std::getline(stream, line)) {
		if (line.find("#include") == 0) {
			// Parse the include (e.g., #include "lighting.glsl")
			const size_t start = line.find('"');
			const size_t end = line.rfind('"');

			if (start != std::string::npos && end != std::string::npos && start != end) {
				std::string_view includeFile{line.data() + start + 1, end - start - 1};

				// Prevent cyclic includes
				if (includedFiles.contains(includeFile.data())) {
					continue;
				}
				includedFiles.emplace(includeFile);
				// Load included file
				auto includePath = shaderRoot / includeFile;
				preprocess(fs::readFile(includePath), output, includedFiles);
			}
		} else {
			output += line;
			output += '\n';
		}
	}
}

Shader::Shader() {
	mHandle = glCreateProgram();
}

Shader::~Shader() {
	if (mHandle)
		glDeleteProgram(mHandle);
}

Shader::Shader(Shader&& other) noexcept
	: mHandle(std::exchange(other.mHandle, 0)) {
}

Shader& Shader::operator=(Shader&& other) noexcept {
	if (this == &other)
		return *this;

	if (mHandle)
		glDeleteProgram(mHandle);

	mHandle = std::exchange(other.mHandle, 0);
	return *this;
}

void Shader::bind() const {
	glUseProgram(mHandle);
}

void Shader::attachStage(const ShaderStage& stage) const {
	glAttachShader(mHandle, stage.handle());
}

void Shader::link() const {
	glLinkProgram(mHandle);
	checkLinkErrors();
}

void Shader::checkLinkErrors() const {
	int32_t success;
	glGetProgramiv(mHandle, GL_LINK_STATUS, &success);

	if (!success) {
		std::string infoLog;
		int32_t maxLength = 0;

		glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &maxLength);
		infoLog.resize(maxLength);

		glGetProgramInfoLog(mHandle, maxLength, nullptr, infoLog.data());

		// The program is useless now. So delete it.
		glDeleteProgram(mHandle);

		throw std::runtime_error(std::string("Linking error:\n") + infoLog);
	}
}

int32_t Shader::getUniformLocation(const std::string_view name) {
	if (const auto it = mUniformLocations.find(name); it != mUniformLocations.end())
		return it->second;

	const int32_t location = glGetUniformLocation(mHandle, name.data());
	mUniformLocations.emplace(std::string(name), location);

	return location;
}

void Uniform::setBool(const int32_t location, const bool value) {
	glUniform1i(location, static_cast<int>(value));
}

void Uniform::setInt(const int32_t location, const int32_t value) {
	glUniform1i(location, value);
}

void Uniform::setUint(const int32_t location, const uint32_t value) {
	glUniform1ui(location, value);
}

void Uniform::setFloat(const int32_t location, const float value) {
	glUniform1f(location, value);
}

void Uniform::setFloatArray(const int32_t location, const float* values, const uint32_t count) {
	glUniform1fv(location, static_cast<int32_t>(count), values);
}

void Uniform::setVec2(const int32_t location, const glm::vec2& value) {
	glUniform2fv(location, 1, &value[0]);
}

void Uniform::setVec2(const int32_t location, const float x, const float y) {
	glUniform2f(location, x, y);
}

void Uniform::setVec3(const int32_t location, const glm::vec3& value) {
	glUniform3fv(location, 1, &value[0]);
}

void Uniform::setVec3(const int32_t location, const float x, const float y, const float z) {
	glUniform3f(location, x, y, z);
}

void Uniform::setVec4(const int32_t location, const glm::vec4& value) {
	glUniform4fv(location, 1, &value[0]);
}

void Uniform::setVec4(const int32_t location, const float x, const float y, const float z,
                      const float w) {
	glUniform4f(location, x, y, z, w);
}

void Uniform::setMat2(const int32_t location, const glm::mat2& mat) {
	glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Uniform::setMat3(const int32_t location, const glm::mat3& mat) {
	glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Uniform::setMat4(const int32_t location, const glm::mat4& mat) {
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Uniform::setMat4Array(const int32_t location, const glm::mat4* matrices, const size_t count) {
	glUniformMatrix4fv(location, static_cast<int32_t>(count), GL_FALSE, glm::value_ptr(matrices[0]));
}

std::string ShaderLoader::load(const std::string_view file) {
	static const std::filesystem::path root = CONFIG_MANAGER.get<std::string>("path.shader");
	const auto path = root / file;
	return fs::readFile(path);
}
