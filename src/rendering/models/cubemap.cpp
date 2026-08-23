#include "cubemap.hpp"
#include "../mesh/mesh.hpp"
#include "../mesh/vertex.hpp"
#include "../texture.hpp"
#include "../material/material.hpp"

Model::Cubemap::Cubemap(std::vector<std::string>& faces) {
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

	std::vector<MaterialTexture> textures;
	textures.reserve(faces.size());

	for (const auto& face : faces) {
		textures.emplace_back(face, TextureType::Albedo, nullptr);
	}
	Material material;
	material.textureTarget = TextureTarget::TextureCubeMap;
	material.textures = std::move(textures);

	mMaterial[0] = std::move(material);
}

Model::Cubemap::~Cubemap() = default;

MeshMap& Model::Cubemap::meshes() {
	return mMeshes;
}

MaterialMap& Model::Cubemap::material() {
	return mMaterial;
}

