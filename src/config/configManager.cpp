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
		set("camera.ubo.binding", config["camera"]["ubo"]["binding"].value_or(0));
		set("camera.ubo.blockName", std::string(config["camera"]["ubo"]["blockName"].value_or("CameraBlock")));

		set("light.max_directional", config["light"]["max_directional"].value_or(4));
		set("light.max_point", config["light"]["max_point"].value_or(4));
		set("light.max_spot", config["light"]["max_spot"].value_or(4));
		set("light.ubo.binding",config["light"]["ubo"]["binding"].value_or(1));
		set("light.ubo.blockName", std::string(config["light"]["ubo"]["blockName"].value_or("LightBlock")));

		set("shadow.map.width", config["shadow"]["map"]["width"].value_or(1024));
		set("shadow.map.height", config["shadow"]["map"]["height"].value_or(1024));
		set("shadow.map.slot", config["shadow"]["map"]["slot"].value_or(1));
		set("shadow.ubo.binding", config["shadow"]["ubo"]["binding"].value_or(2));
		set("shadow.ubo.blockName", std::string(config["shadow"]["ubo"]["blockName"].value_or("ShadowBlock")));
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

		set("ssao.slot", config["ssao"]["slot"].value_or(24));
		set("ssao.kernelSize", config["ssao"]["kernel_size"].value_or(32));
		set("ssao.radius", config["ssao"]["radius"].value_or(1.0f));
		set("ssao.bias", config["ssao"]["bias"].value_or(0.005f));
		set("ssao.intensity", config["ssao"]["intensity"].value_or(1.0f));
		set("ssao.ubo.binding", config["ssao"]["ubo"]["binding"].value_or(3));
		set("ssao.ubo.blockName", std::string(config["ssao"]["ubo"]["blockName"].value_or("SSAOBlock")));
		set("ssao.noise.size", config["ssao"]["noise"]["size"].value_or(128));
		set("ssao.noise.slot", config["ssao"]["noise"]["slot"].value_or(18));

		set("PBR.envMap.size", config["pbr"]["envmap_size"].value_or(512));
		set("PBR.irradianceMap.size", config["pbr"]["irradiance_map_size"].value_or(32));
		set("PBR.prefilterMap.size", config["pbr"]["prefilter_map_size"].value_or(512));
		set("PBR.brdfLUT.size", config["pbr"]["brdf_lut_size"].value_or(512));
		set("PBR.irradianceMap.slot", config["pbr"]["texture_slots"]["irradiance_map"].value_or(9));
		set("PBR.prefilterMap.slot", config["pbr"]["texture_slots"]["prefilter_map"].value_or(10));
		set("PBR.brdfLUT.slot", config["pbr"]["texture_slots"]["brdf_lut"].value_or(11));
		set("PBR.albedo.slot", config["pbr"]["texture_slots"]["albedo"].value_or(12));
		set("PBR.normal.slot", config["pbr"]["texture_slots"]["normal"].value_or(13));
		set("PBR.roughnessMetallic.slot", config["pbr"]["texture_slots"]["rm"].value_or(14));
		set("PBR.ao.slot", config["pbr"]["texture_slots"]["ao"].value_or(15));
		set("PBR.emissive.slot", config["pbr"]["texture_slots"]["emissive"].value_or(16));
		set("PBR.height.slot", config["pbr"]["texture_slots"]["height"].value_or(17));

		set("gBuffer.position.index", config["gbuffer"]["indices"]["position"].value_or(0));
		set("gBuffer.position.slot", config["gbuffer"]["texture_slots"]["position"].value_or(19));
		set("gBuffer.normal.index", config["gbuffer"]["indices"]["normal"].value_or(1));
		set("gBuffer.normal.slot", config["gbuffer"]["texture_slots"]["normal"].value_or(20));
		set("gBuffer.albedo.index", config["gbuffer"]["indices"]["albedo"].value_or(2));
		set("gBuffer.albedo.slot", config["gbuffer"]["texture_slots"]["albedo"].value_or(21));
		set("gBuffer.orm.index", config["gbuffer"]["indices"]["orm"].value_or(3));
		set("gBuffer.orm.slot", config["gbuffer"]["texture_slots"]["orm"].value_or(22));
		set("gBuffer.depth.index", config["gbuffer"]["indices"]["depth"].value_or(4));
		set("gBuffer.depth.slot", config["gbuffer"]["texture_slots"]["depth"].value_or(23));
	} catch (const toml::parse_error& err) {
		throw std::runtime_error(std::string("Parsing failed: ") + err.what());
	}
}
