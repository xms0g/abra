#include "configManager.h"
#include "toml++/toml.hpp"
#include "../io/filesystem.hpp"

void ConfigManager::load(const std::string& filepath) {
	try {
		auto config = toml::parse_file(fs::path(filepath));

		paths.shader_dir = config["paths"]["shader_dir"].value_or("shaders/");
		paths.asset_dir = config["paths"]["asset_dir"].value_or("assets/");

		window.title = config["window"]["title"].value_or("Test Engine");
		window.fullscreen = config["window"]["fullscreen"].value_or(false);
		window.width = config["window"]["width"].value_or(1280);
		window.height = config["window"]["height"].value_or(720);

		msaa.sample_count = config["msaa"]["sample_count"].value_or(4);

		camera.yaw = config["camera"]["yaw"].value_or(-90.0f);
		camera.pitch = config["camera"]["pitch"].value_or(0.0f);
		camera.speed = config["camera"]["speed"].value_or(1.0f);
		camera.sensitivity = config["camera"]["sensitivity"].value_or(0.05f);
		camera.zoom = config["camera"]["zoom"].value_or(45.0f);
		camera.znear = config["camera"]["znear"].value_or(0.1f);
		camera.zfar = config["camera"]["zfar"].value_or(1000.0f);
		camera.ubo_binding = config["camera"]["ubo_binding"].value_or(0);
		camera.block_name = config["camera"]["block_name"].value_or("CameraBlock");

		light.max_directional = config["light"]["max_directional"].value_or(4);
		light.max_point = config["light"]["max_point"].value_or(4);
		light.max_spot = config["light"]["max_spot"].value_or(4);
		light.ubo_binding = config["light"]["ubo_binding"].value_or(1);
		light.block_name = config["light"]["block_name"].value_or("LightBlock");

		shadow.map_width = config["shadow"]["map_width"].value_or(1024);
		shadow.map_height = config["shadow"]["map_height"].value_or(1024);
		shadow.texture_slot = config["shadow"]["texture_slot"].value_or(1);
		shadow.ubo_binding = config["shadow"]["ubo_binding"].value_or(2);
		shadow.block_name = config["shadow"]["block_name"].value_or("ShadowBlock");
		shadow.directional.height = config["shadow"]["directional"]["height"].value_or(1024);
		shadow.directional.nearPlane = config["shadow"]["directional"]["near"].value_or(0.1f);
		shadow.directional.farPlane = config["shadow"]["directional"]["far"].value_or(1000.0f);
		shadow.directional.left = config["shadow"]["directional"]["left"].value_or(-1.0f);
		shadow.directional.right = config["shadow"]["directional"]["right"].value_or(1.0f);
		shadow.directional.bottom = config["shadow"]["directional"]["bottom"].value_or(-1.0f);
		shadow.directional.top = config["shadow"]["directional"]["top"].value_or(1.0f);
		shadow.omnidirectional.nearPlane = config["shadow"]["omnidirectional"]["near"].value_or(0.1f);
		shadow.omnidirectional.farPlane = config["shadow"]["omnidirectional"]["far"].value_or(1000.0f);
		shadow.omnidirectional.fovy = config["shadow"]["omnidirectional"]["fovy"].value_or(90.0f);
		shadow.perspective.nearPlane = config["shadow"]["perspective"]["near"].value_or(1000.0f);
		shadow.perspective.farPlane = config["shadow"]["perspective"]["far"].value_or(10000.0f);

		ssao.kernelSize = config["ssao"]["kernel_size"].value_or(32);
		ssao.radius = config["ssao"]["radius"].value_or(1.0f);
		ssao.bias = config["ssao"]["bias"].value_or(0.005f);
		ssao.intensity = config["ssao"]["intensity"].value_or(1.0f);
		ssao.ubo_binding = config["ssao"]["ubo_binding"].value_or(3);
		ssao.block_name = config["ssao"]["block_name"].value_or("SSAOBlock");
		ssao.noise.textureSize = config["ssao"]["noise"]["texture_size"].value_or(128);
		ssao.noise.textureSlot = config["ssao"]["noise"]["texture_slot"].value_or(18);

		PBR.envMap.size = config["pbr"]["envmap_size"].value_or(512);
		PBR.irradianceMap.size = config["pbr"]["irradiance_map_size"].value_or(32);
		PBR.irradianceMap.textureSlot = config["pbr"]["irradiance_map_texture_slot"].value_or(9);
		PBR.prefilterMap.size = config["pbr"]["prefilter_map_size"].value_or(512);
		PBR.prefilterMap.textureSlot = config["pbr"]["prefilter_map_texture_slot"].value_or(10);
		PBR.brdfLUT.size = config["pbr"]["brdf_lut_size"].value_or(512);
		PBR.brdfLUT.textureSlot = config["pbr"]["brdf_lut_texture_slot"].value_or(11);

		PBR.albedoTextureSlot = config["pbr"]["texture_slots"]["albedo"].value_or(12);
		PBR.normalTextureSlot = config["pbr"]["texture_slots"]["normal"].value_or(13);
		PBR.roughnessMetallicTextureSlot = config["pbr"]["texture_slots"]["rm"].value_or(14);
		PBR.aoTextureSlot = config["pbr"]["texture_slots"]["ao"].value_or(15);
		PBR.emissiveTextureSlot = config["pbr"]["texture_slots"]["emissive"].value_or(16);
		PBR.heightTextureSlot = config["pbr"]["texture_slots"]["height"].value_or(17);

		gBuffer.position.textureIdx = config["gbuffer"]["indices"]["position"].value_or(0);
		gBuffer.position.textureSlot = config["gbuffer"]["texture_slots"]["position"].value_or(19);
		gBuffer.normal.textureIdx = config["gbuffer"]["indices"]["normal"].value_or(1);
		gBuffer.normal.textureSlot = config["gbuffer"]["texture_slots"]["normal"].value_or(20);
		gBuffer.albedo.textureIdx = config["gbuffer"]["indices"]["albedo"].value_or(2);
		gBuffer.albedo.textureSlot = config["gbuffer"]["texture_slots"]["albedo"].value_or(21);
		gBuffer.orm.textureIdx = config["gbuffer"]["indices"]["orm"].value_or(3);
		gBuffer.orm.textureSlot = config["gbuffer"]["texture_slots"]["orm"].value_or(22);
		gBuffer.depth.textureIdx = config["gbuffer"]["indices"]["depth"].value_or(4);
		gBuffer.depth.textureSlot = config["gbuffer"]["texture_slots"]["depth"].value_or(23);
	} catch (const toml::parse_error& err) {
		throw std::runtime_error(std::string("Parsing failed: ") + err.what());
	}
}

ConfigManager& ConfigManager::instance() {
	static ConfigManager ins;
	return ins;
}
