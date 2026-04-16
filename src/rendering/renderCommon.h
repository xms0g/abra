#pragma once
#include <unordered_map>
#include <vector>

struct InstanceGroup;
struct RenderableObject;
struct RenderContext;
struct EntityCore;
struct Material;
struct Texture;
class Mesh;
class Shader;
class Entity;

namespace RenderCommon {
void forward(const std::vector<RenderableObject>& objects);

void instanced(const std::vector<InstanceGroup>& objects);

void setupTransform(const EntityCore& entity, const Shader& shader);

void setupMaterial(const EntityCore& entity, const Material& material, const Shader& shader);

void drawMesh(const Mesh& mesh);

void drawQuad(uint32_t sceneTexture, uint32_t VAO);

void bindTextures(const std::vector<Texture>& textures, const Shader& shader);

void bindShadowMaps(const RenderContext& ctx);
}
