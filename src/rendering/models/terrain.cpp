#include "terrain.h"
#include "image/stb_image.h"
#include "../mesh/mesh.h"
#include "../mesh/vertex.hpp"
#include "../texture/texture.h"
#include "../material/material.hpp"
#include "../../io/filesystem.hpp"
#include "../../config/configManager.h"

Model::Terrain::Terrain(
	glm::vec3 color,
	bool unlit,
	const std::string& diffuseTexture,
	const std::string& specularTexture,
	const std::string& normalTexture,
	const std::string& heightTexture) {
	std::string path = fs::resolvePath(CONFIG_MANAGER.get<std::string>("path.asset") + heightTexture);

	int32_t width, height;
	Texture::info(path, width, height);

	std::vector<float> v;

	uint32_t rez = 20;
	auto rezf = static_cast<float>(rez);

	for (uint32_t i = 0; i <= rez - 1; ++i) {
		for (uint32_t j = 0; j <= rez - 1; ++j) {
			v.push_back(-width / 2.0f + width * i / rezf); // v.x
			v.push_back(0.0f); // v.y
			v.push_back(-height / 2.0f + height * j / rezf); // v.z
			v.push_back(i / rezf); // u
			v.push_back(j / rezf); // v

			v.push_back(-width / 2.0f + width * (i + 1) / rezf); // v.x
			v.push_back(0.0f); // v.y
			v.push_back(-height / 2.0f + height * j / rezf); // v.z
			v.push_back((i + 1) / rezf); // u
			v.push_back(j / rezf); // v

			v.push_back(-width / 2.0f + width * i / rezf); // v.x
			v.push_back(0.0f); // v.y
			v.push_back(-height / 2.0f + height * (j + 1) / rezf); // v.z
			v.push_back(i / rezf); // u
			v.push_back((j + 1) / rezf); // v

			v.push_back(-width / 2.0f + width * (i + 1) / rezf); // v.x
			v.push_back(0.0f); // v.y
			v.push_back(-height / 2.0f + height * (j + 1) / rezf); // v.z
			v.push_back((i + 1) / rezf); // u
			v.push_back((j + 1) / rezf); // v
		}
	}
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (uint32_t i = 0; i < v.size(); i += 5) {
		Vertex vertex{};
		vertex.position = glm::vec3(v[i], v[i + 1], v[i + 2]);
		vertex.texcoord = glm::vec2(v[i + 3], v[i + 4]);
		vertices.emplace_back(vertex);
	}

	mMeshes[0].emplace_back(vertices, indices);

	uint32_t flags{0};
	std::vector<Texture> textures;
	if (!heightTexture.empty()) {
		flags |= HAS_HEIGHT_MAP;
		textures.emplace_back(0, HEIGHT,TextureTarget::Texture2D, heightTexture);
	}

	Material material;
	material.flags = flags;
	material.textureTarget = TextureTarget::Texture2D;
	material.textures = std::move(textures);

	mMaterial[0] = std::move(material);
}

Model::Terrain::~Terrain() = default;

MeshMap& Model::Terrain::meshes() {
	return mMeshes;
}

MaterialMap& Model::Terrain::material() {
	return mMaterial;
}
