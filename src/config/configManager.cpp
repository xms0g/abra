#include "configManager.h"
#include "toml++/toml.hpp"
#include "../io/filesystem.hpp"

ConfigManager& ConfigManager::instance() {
	static ConfigManager ins;
	return ins;
}

void ConfigManager::load(const std::string_view filepath) {
	try {
		auto config = toml::parse_file(fs::resolvePath(filepath));

		set("path.shader", std::string(config["paths"]["shader_dir"].value_or("")));
		set("path.asset", std::string(config["paths"]["asset_dir"].value_or("")));
		set("path.scene", std::string(config["paths"]["scene_dir"].value_or("")));

		set("window.title", std::string(config["window"]["title"].value_or("Test Engine")));
		set("window.fullscreen", config["window"]["fullscreen"].value_or(false));
		set("window.width", config["window"]["width"].value_or(1280));
		set("window.height", config["window"]["height"].value_or(720));

		set("hdr.enabled", config["hdr"]["enabled"].value_or(true));

		set("msaa.enabled", config["msaa"]["enabled"].value_or(true));
		set("msaa.sample_count", config["msaa"]["sample_count"].value_or(4));

		set("camera.yaw", config["camera"]["yaw"].value_or(-90.0f));
		set("camera.pitch", config["camera"]["pitch"].value_or(0.0f));
		set("camera.speed", config["camera"]["speed"].value_or(1.0f));
		set("camera.sensitivity", config["camera"]["sensitivity"].value_or(0.05f));
		set("camera.zoom", config["camera"]["zoom"].value_or(45.0f));
		set("camera.znear", config["camera"]["znear"].value_or(0.1f));
		set("camera.zfar", config["camera"]["zfar"].value_or(1000.0f));
		set("camera.ubo_binding", config["camera"]["ubo_binding"].value_or(0));
		set("camera.block_name", std::string(config["camera"]["block_name"].value_or("CameraBlock")));

		set("light.max_directional", config["light"]["max_directional"].value_or(4));
		set("light.max_point", config["light"]["max_point"].value_or(4));
		set("light.max_spot", config["light"]["max_spot"].value_or(4));
		set("light.ubo_binding",config["light"]["ubo_binding"].value_or(1));
		set("light.block_name", std::string(config["light"]["block_name"].value_or("LightBlock")));

		set("shadow.map_width", config["shadow"]["map_width"].value_or(1024));
		set("shadow.map_height", config["shadow"]["map_height"].value_or(1024));
		set("shadow.texture_slot", config["shadow"]["texture_slot"].value_or(1));
		set("shadow.ubo_binding", config["shadow"]["ubo_binding"].value_or(2));
		set("shadow.block_name", std::string(config["shadow"]["block_name"].value_or("ShadowBlock")));
		set("shadow.directional.height", config["shadow"]["directional"]["height"].value_or(1.0f));
		set("shadow.directional.nearPlane", config["shadow"]["directional"]["near"].value_or(0.1f));
		set("shadow.directional.farPlane", config["shadow"]["directional"]["far"].value_or(1000.0f));
		set("shadow.directional.left", config["shadow"]["directional"]["left"].value_or(-1.0f));
		set("shadow.directional.right", config["shadow"]["directional"]["right"].value_or(1.0f));
		set("shadow.directional.bottom", config["shadow"]["directional"]["bottom"].value_or(-1.0f));
		set("shadow.directional.top", config["shadow"]["directional"]["top"].value_or(1.0f));
		set("shadow.omnidirectional.nearPlane", config["shadow"]["omnidirectional"]["near"].value_or(0.1f));
		set("shadow.omnidirectional.farPlane", config["shadow"]["omnidirectional"]["far"].value_or(1000.0f));
		set("shadow.omnidirectional.fovy", config["shadow"]["omnidirectional"]["fovy"].value_or(90.0f));
		set("shadow.perspective.nearPlane", config["shadow"]["perspective"]["near"].value_or(1000.0f));
		set("shadow.perspective.farPlane", config["shadow"]["perspective"]["far"].value_or(10000.0f));

		set("ssao.textureSlot", config["ssao"]["texture_slot"].value_or(24));
		set("ssao.kernelSize", config["ssao"]["kernel_size"].value_or(32));
		set("ssao.radius", config["ssao"]["radius"].value_or(1.0f));
		set("ssao.bias", config["ssao"]["bias"].value_or(0.005f));
		set("ssao.intensity", config["ssao"]["intensity"].value_or(1.0f));
		set("ssao.ubo_binding", config["ssao"]["ubo_binding"].value_or(3));
		set("ssao.block_name", std::string(config["ssao"]["block_name"].value_or("SSAOBlock")));
		set("ssao.noise.textureSize", config["ssao"]["noise"]["texture_size"].value_or(128));
		set("ssao.noise.textureSlot", config["ssao"]["noise"]["texture_slot"].value_or(18));

		set("PBR.envMap.size", config["pbr"]["envmap_size"].value_or(512));
		set("PBR.irradianceMap.size", config["pbr"]["irradiance_map_size"].value_or(32));
		set("PBR.prefilterMap.size", config["pbr"]["prefilter_map_size"].value_or(512));
		set("PBR.brdfLUT.size", config["pbr"]["brdf_lut_size"].value_or(512));
		set("PBR.irradianceMap.textureSlot", config["pbr"]["texture_slots"]["irradiance_map"].value_or(9));
		set("PBR.prefilterMap.textureSlot", config["pbr"]["texture_slots"]["prefilter_map"].value_or(10));
		set("PBR.brdfLUT.textureSlot", config["pbr"]["texture_slots"]["brdf_lut"].value_or(11));
		set("PBR.albedo.textureSlot", config["pbr"]["texture_slots"]["albedo"].value_or(12));
		set("PBR.normal.textureSlot", config["pbr"]["texture_slots"]["normal"].value_or(13));
		set("PBR.roughnessMetallic.textureSlot", config["pbr"]["texture_slots"]["rm"].value_or(14));
		set("PBR.ao.textureSlot", config["pbr"]["texture_slots"]["ao"].value_or(15));
		set("PBR.emissive.textureSlot", config["pbr"]["texture_slots"]["emissive"].value_or(16));
		set("PBR.height.textureSlot", config["pbr"]["texture_slots"]["height"].value_or(17));

		set("gBuffer.position.textureIdx", config["gbuffer"]["indices"]["position"].value_or(0));
		set("gBuffer.position.textureSlot", config["gbuffer"]["texture_slots"]["position"].value_or(19));
		set("gBuffer.normal.textureIdx", config["gbuffer"]["indices"]["normal"].value_or(1));
		set("gBuffer.normal.textureSlot", config["gbuffer"]["texture_slots"]["normal"].value_or(20));
		set("gBuffer.albedo.textureIdx", config["gbuffer"]["indices"]["albedo"].value_or(2));
		set("gBuffer.albedo.textureSlot", config["gbuffer"]["texture_slots"]["albedo"].value_or(21));
		set("gBuffer.orm.textureIdx", config["gbuffer"]["indices"]["orm"].value_or(3));
		set("gBuffer.orm.textureSlot", config["gbuffer"]["texture_slots"]["orm"].value_or(22));
		set("gBuffer.depth.textureIdx", config["gbuffer"]["indices"]["depth"].value_or(4));
		set("gBuffer.depth.textureSlot", config["gbuffer"]["texture_slots"]["depth"].value_or(23));
	} catch (const toml::parse_error& err) {
		throw std::runtime_error(std::string("Parsing failed: ") + err.what());
	}
}
