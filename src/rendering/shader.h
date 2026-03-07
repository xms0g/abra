#pragma once
#include <string>
#include <unordered_set>
#include "glm/glm.hpp"

class Shader {
public:
	Shader(const char* vs, const char* fs, const char* gs = nullptr);

	~Shader();

	Shader(const Shader&) = delete;

	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;

	Shader& operator=(Shader&& other) noexcept;

	[[nodiscard]] uint32_t ID() const { return mID; }

	// use/activate the shader
	void activate() const;

	// utility uniform functions
	void setBool(const std::string& name, bool value) const;

	void setInt(const std::string& name, int value) const;

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

private:
	std::string loadFile(const char* fn);

	std::string preprocess(const std::string& source,
	                       const char* fileName,
	                       std::unordered_set<std::string>& includedFiles);

	uint32_t compileShader(const std::string& source, const char* fn, uint32_t type);

	uint32_t linkShader(uint32_t vertex, uint32_t fragment, uint32_t geometry = 0);

	// the program ID
	uint32_t mID{};
};
