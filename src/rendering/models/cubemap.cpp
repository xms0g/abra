#include "cubemap.h"
#include "../mesh/mesh.h"
#include "../texture/texture.h"
#include "../material/material.hpp"
#include "../../config/config.hpp"
#include "../../io/filesystem.hpp"

Models::Cubemap::Cubemap(std::span<const char* const> faces) {
	constexpr float v[]= {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	std::vector<Vertex> vertices;
	for (uint32_t i = 0; i < 36; ++i) {
		Vertex vertex{};
		vertex.position = glm::vec3(v[i*3], v[i*3+1], v[i*3+2]);
		vertices.emplace_back(vertex);
	}

	std::vector<uint32_t> indices;
	mMeshes[0].emplace_back(vertices, indices);
	mMeshes.at(0).at(0).uploadToGPU();

	std::vector<std::string> facesVec;
	facesVec.reserve(faces.size());

	for (const auto& face : faces) {
		facesVec.emplace_back(fs::path(ASSET_DIR + face));
	}


	std::vector<Texture> textures;
	textures.emplace_back(texture::loadCubemap(facesVec), ALBEDO, "skybox","skybox.jpg");
	mMaterial[0] = {0, glm::vec3(), 0.0f, textures};
}

Models::Cubemap::~Cubemap() = default;

[[nodiscard]] MeshMap* Models::Cubemap::meshes() { return &mMeshes; }

[[nodiscard]] const MaterialMap* Models::Cubemap::material() const { return &mMaterial; }

