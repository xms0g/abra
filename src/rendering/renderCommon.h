#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

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
void forward(const RenderContext& ctx, const std::vector<RenderableObject>& objects);

void instanced(const RenderContext& ctx, const std::vector<InstanceGroup>& objects);

void setupTransform(size_t entityID, const RenderContext& ctx, const Shader& shader);

void setupMaterial(size_t entityID, uint32_t materialIdx, const RenderContext& ctx, const Shader& shader);

void drawMesh(uint32_t vao, size_t vertexCount, size_t indexCount);

void drawQuad(uint32_t sceneTexture, uint32_t vao);

void bindTextures(uint32_t flags, const std::vector<uint32_t>& textures, const Shader& shader);

void bindShadowMaps(const RenderContext& ctx);
}
