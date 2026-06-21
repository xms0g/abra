#pragma once
#include <string>

class ConfigManager {
public:
	ConfigManager(const ConfigManager&) = delete;

	ConfigManager& operator=(const ConfigManager&) = delete;

	static ConfigManager& instance();

	struct {
		std::string shader_dir;
		std::string asset_dir;
	} paths;

	struct {
		std::string title;
		bool fullscreen;
		uint32_t width, height;
	} window{};

	struct {
		int32_t sample_count;
	} msaa{};

	struct {
		float yaw, pitch, speed, sensitivity, zoom, znear, zfar;
		uint32_t ubo_binding;
		std::string block_name;
	} camera;

	struct {
		uint32_t max_directional;
		uint32_t max_point;
		uint32_t max_spot;
		uint32_t ubo_binding;
		std::string block_name;
	} light;

	struct {
		uint32_t map_width, map_height;
		int32_t texture_slot;
		uint32_t ubo_binding;
		std::string block_name;

		struct {
			float height, nearPlane, farPlane, left, right, bottom, top;
		} directional;

		struct {
			float nearPlane, farPlane, fovy;
		} omnidirectional;

		struct {
			float nearPlane, farPlane;
		} perspective;
	} shadow;

	struct {
		int32_t kernelSize;
		float radius;
		float bias;
		float intensity;

		uint32_t ubo_binding;
		std::string block_name;

		int32_t textureSlot;

		struct {
			int32_t textureSize;
			int32_t textureSlot;
		} noise;

	} ssao{};

	struct {
		struct {
			uint32_t size;
		} envMap;

		struct {
			uint32_t size;
			int32_t textureSlot;
		} irradianceMap;

		struct {
			uint32_t size;
			int32_t textureSlot;
		} prefilterMap;

		struct {
			uint32_t size;
			int32_t textureSlot;
		} brdfLUT;

		int32_t albedoTextureSlot;
		int32_t normalTextureSlot;
		int32_t roughnessMetallicTextureSlot;
		int32_t aoTextureSlot;
		int32_t emissiveTextureSlot;
		int32_t heightTextureSlot;
	} PBR{};

	struct {
		struct {
			int32_t textureIdx;
			int32_t textureSlot;
		} position;

		struct {
			int32_t textureIdx;
			int32_t textureSlot;
		} normal;

		struct {
			int32_t textureIdx;
			int32_t textureSlot;
		} albedo;

		struct {
			int32_t textureIdx;
			int32_t textureSlot;
		} orm;

		struct {
			int32_t textureIdx;
			int32_t textureSlot;
		} depth;

	} gBuffer{};

	void load(const std::string& filepath);

private:
	explicit ConfigManager() = default;

	~ConfigManager() = default;
};
