#pragma once
#include <unordered_map>
#include <vector>
#include <array>

struct Texture;
class Mesh;
class Shader;
class Entity;

namespace RenderCommon {
void setupTransform(const Entity& entity, const Shader& shader);

void setupMaterial(const Entity& entity, const Shader& shader);

void drawMeshes(const std::vector<Mesh>& meshes);

void drawQuad(uint32_t sceneTexture, uint32_t VAO);

void bindTextures(const std::vector<Texture>& textures, const Shader& shader);

void unbindTextures(const std::vector<Texture>& textures);

void bindShadowMaps(const std::array<uint32_t, 3>& shadowMaps);
}
