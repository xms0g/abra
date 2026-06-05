#pragma once
#include <unordered_map>
#include <vector>
#include <span>

struct TextureBinding;
struct InstanceGroup;
struct RenderableObject;
struct RenderContext;
struct Material;
struct Texture;
class Shader;

namespace RenderCommand {
void forward(const RenderContext& ctx, const std::vector<RenderableObject>& objects);

void instanced(const RenderContext& ctx, const std::vector<InstanceGroup>& objects);

void setupTransform(size_t entityID, const RenderContext& ctx, const Shader& shader);

void setupMaterial(
	size_t entityID,
	uint32_t materialIdx,
	uint32_t textureOffset,
	size_t textureCount,
	const RenderContext& ctx,
	const Shader& shader);

void setTextureUnits(const std::vector<TextureBinding>& textures, const Shader& shader);

void drawMesh(uint32_t vao, size_t vertexCount, size_t indexCount);

void drawQuad(uint32_t vao);

void drawQuad(uint32_t vao, std::span<uint32_t> textures);

void bindShadowMaps(const RenderContext& ctx);
}
