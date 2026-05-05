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

void setupTransform(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const Shader& shader);

void setupMaterial(const Material& material, const Shader& shader, float heightScale);

void drawMesh(const Mesh& mesh);

void drawQuad(uint32_t sceneTexture, uint32_t VAO);

void bindTextures(const Material& material, const Shader& shader);

void bindShadowMaps(const RenderContext& ctx);
}
