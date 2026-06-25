#pragma once
#include <string>
#include <unordered_set>
#include "glm/glm.hpp"

struct ShaderResource;

struct ShaderResource {
	uint32_t handle{0};

	ShaderResource() = default;

	explicit ShaderResource(const char** code, const std::string& fn, uint32_t type);

	ShaderResource(const ShaderResource& other) = delete;

	ShaderResource& operator=(const ShaderResource& other) = delete;

	ShaderResource(ShaderResource&& other) noexcept;

	ShaderResource& operator=(ShaderResource&& other) noexcept;

	~ShaderResource();

	void attach(uint32_t programID) const;

	void checkCompileErrors(const std::string& fn) const;
};

class Shader {
public:
	Shader(
		const std::string& vs,
		const std::string& fs,
		const std::string& gs = "",
		const std::string& tcs = "",
		const std::string& tes = "");

	~Shader();

	Shader(const Shader&) = delete;

	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;

	Shader& operator=(Shader&& other) noexcept;

	[[nodiscard]]
	uint32_t id() const { return mID; }

	// use/activate the shader
	void activate() const;

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

	static std::string preprocess(const std::string& source);
	static std::string preprocess(const std::string& source, std::unordered_set<std::string>& includedFiles);

	static ShaderResource compileShader(const std::string& source, const std::string& fn, uint32_t type);

	static void linkShader(
		uint32_t& programID,
		const ShaderResource& vert,
		const ShaderResource& frag,
		const ShaderResource& geo,
		const ShaderResource& tessControl,
		const ShaderResource& tessEval);

	static void checkLinkErrors(uint32_t programID);

private:
	// the program ID
	uint32_t mID{};
};
