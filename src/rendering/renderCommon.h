#pragma once
#include <unordered_map>
#include <vector>

struct RenderableObject;
struct RenderContext;
struct EntityData;
struct Material;
struct Texture;
class Mesh;
class Shader;
class Entity;

namespace RenderCommon {
void forward(const std::vector<RenderableObject>& objects);

void setupTransform(const EntityData& entity, const Shader& shader);

void setupMaterial(const EntityData& entity, const Material& material, const Shader& shader);

void drawMesh(const Mesh& mesh);

void drawQuad(uint32_t sceneTexture, uint32_t VAO);

void bindTextures(const std::vector<Texture>& textures, const Shader& shader);

void unbindTextures(const std::vector<Texture>& textures);

void bindShadowMaps(const RenderContext& ctx);
}
