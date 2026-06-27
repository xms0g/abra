#include "terrain.h"
#include "glad/glad.h"
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
	const char* diffuseTexture,
	const char* specularTexture,
	const char* normalTexture,
	const char* heightTexture) {
	std::string path = fs::path(cfg.get<std::string>("path.asset") + heightTexture);

	int32_t width, height;
	Texture::info(path.c_str(), width, height);

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
	if (heightTexture) {
		flags |= HAS_HEIGHT_MAP;
		textures.emplace_back(0, HEIGHT, heightTexture);
	}

	mMaterial[0] = {.flags = flags | TERRAIN, .textureTarget = GL_TEXTURE_2D, .textures = textures};
}

Model::Terrain::~Terrain() = default;

MeshMap& Model::Terrain::meshes() {
	return mMeshes;
}

MaterialMap& Model::Terrain::material() {
	return mMaterial;
}
