#pragma once
#include <string>
#include <cstdint>

// PATH
constexpr std::string SHADER_DIR = "shaders/";
constexpr std::string ASSET_DIR = "assets/";

// MSAA
constexpr uint32_t MULTISAMPLED_COUNT{4};

// Window
inline uint32_t SCR_WIDTH{0};
inline uint32_t SCR_HEIGHT{0};

// Camera
constexpr float YAW{-90.0f};
constexpr float PITCH{0.0f};
constexpr float SPEED{10.0f};
constexpr float SENSITIVITY{0.1f};
constexpr float ZOOM{45.0f};
constexpr float ZNEAR{0.1f};
constexpr float ZFAR{100.0f};

constexpr uint32_t CAMERA_UBO_BINDING{0};
constexpr auto CAMERA_UBO_BLOCK_NAME = "CameraBlock";
// Light
constexpr uint32_t MAX_DIRECTIONAL_LIGHTS{1};
constexpr uint32_t MAX_POINT_LIGHTS{4};
constexpr uint32_t MAX_SPOT_LIGHTS{4};

constexpr uint32_t LIGHT_UBO_BINDING{1};
constexpr auto LIGHT_UBO_BLOCK_NAME = "LightBlock";
// Shadow
constexpr uint32_t SHADOWMAP_WIDTH{1024};
constexpr uint32_t SHADOWMAP_HEIGHT{1024};
constexpr uint32_t SHADOWMAP_TEXTURE_SLOT{6};

constexpr float SHADOW_DIRECTIONAL_HEIGHT{5.0};
constexpr float SHADOW_DIRECTIONAL_NEAR{1.0f};
constexpr float SHADOW_DIRECTIONAL_FAR{7.5f};
constexpr float SHADOW_DIRECTIONAL_LEFT{-10.0f};
constexpr float SHADOW_DIRECTIONAL_RIGHT{10.0f};
constexpr float SHADOW_DIRECTIONAL_BOTTOM{-10.0f};
constexpr float SHADOW_DIRECTIONAL_TOP{10.0f};
constexpr float SHADOW_OMNIDIRECTIONAL_NEAR{1.0f};
constexpr float SHADOW_OMNIDIRECTIONAL_FAR{25.0f};
constexpr float SHADOW_OMNIDIRECTIONAL_FOVY{90.0f};
constexpr float SHADOW_PERSPECTIVE_NEAR{1.0f};
constexpr float SHADOW_PERSPECTIVE_FAR{25.0f};

constexpr uint32_t SHADOW_UBO_BINDING{2};
constexpr auto SHADOW_UBO_BLOCK_NAME = "ShadowBlock";
// SSAO
constexpr uint32_t SSAO_KERNEL_SIZE{32};
constexpr uint32_t SSAO_NOISE_TEXTURE_SIZE{4};
constexpr float SSAO_RADIUS{0.5f};
constexpr float SSAO_BIAS{0.025f};
constexpr float SSAO_INTENSITY{3.0f};
constexpr uint32_t SSAO_UBO_BINDING{3};
constexpr int32_t SSAO_TEXTURE_SLOT{5};
constexpr auto SSAO_UBO_BLOCK_NAME = "SSAOBlock";
// PBR
constexpr uint32_t PBR_ENVMAP_SIZE{512};

constexpr uint32_t PBR_IRRADIANCE_MAP_SIZE{32};
constexpr uint32_t PBR_IRRADIANCE_MAP_TEXTURE_SLOT{9};

constexpr uint32_t PBR_PREFILTER_MAP_SIZE{128};
constexpr uint32_t PBR_PREFILTER_MAP_TEXTURE_SLOT{10};

constexpr uint32_t PBR_BRDF_LUT_SIZE{512};
constexpr uint32_t PBR_BRDF_LUT_TEXTURE_SLOT{11};

constexpr auto PBR_HDR_TEXTURE = "textures/hdr/newport_loft.hdr";
// Texture Slots
constexpr uint32_t PBR_ALBEDO_TEXTURE_SLOT{0};
constexpr uint32_t PBR_NORMAL_TEXTURE_SLOT{1};
constexpr uint32_t PBR_RM_TEXTURE_SLOT{2};
constexpr uint32_t PBR_AO_TEXTURE_SLOT{3};
constexpr uint32_t PBR_EMISSIVE_TEXTURE_SLOT{4};
constexpr uint32_t PBR_HEIGHT_TEXTURE_SLOT{5};

