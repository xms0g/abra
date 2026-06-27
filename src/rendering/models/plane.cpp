#include "plane.h"
#include "glad/glad.h"
#include "../mesh/mesh.h"
#include "../mesh/vertex.hpp"
#include "../texture/texture.h"
#include "../material/material.hpp"

Model::Plane::Plane(
	glm::vec3 color,
	bool unlit,
	const char* diffuseTexture,
	const char* specularTexture,
	const char* normalTexture,
	const char* heightTexture) {
	constexpr float v[] = {
		// Positions        // Normals		   // Texture Coords
		5.0f, -0.5f, 5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f, // top right
		-5.0f, -0.5f, 5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // top left
		-5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f, // bottom left
		5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f // bottom right
	};

	std::vector<uint32_t> indices = {
		0, 2, 1,
		0, 3, 2
	};

	std::vector<Vertex> vertices;
	for (uint32_t i = 0; i < 8 * 4; i += 8) {
		Vertex vertex{};
		vertex.position = glm::vec3(v[i], v[i + 1], v[i + 2]);
		vertex.normal = glm::vec3(v[i + 3], v[i + 4], v[i + 5]);
		vertex.texcoord = glm::vec2(v[i + 6], v[i + 7]);

		vertices.emplace_back(vertex);
	}

	// Calculate tangents and bitangents
	for (uint32_t i = 0; i < indices.size(); i += 3) {
		uint32_t i0 = indices[i];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		glm::vec3 pos0 = vertices[i0].position;
		glm::vec3 pos1 = vertices[i1].position;
		glm::vec3 pos2 = vertices[i2].position;

		glm::vec2 uv0 = vertices[i0].texcoord;
		glm::vec2 uv1 = vertices[i1].texcoord;
		glm::vec2 uv2 = vertices[i2].texcoord;

		glm::vec3 edge1 = pos1 - pos0;
		glm::vec3 edge2 = pos2 - pos0;
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		auto tangent = glm::vec3(
			f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
			f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
			f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z));

		auto bitangent = glm::vec3(
			f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
			f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
			f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z));

		vertices[i0].tangent = vertices[i1].tangent = vertices[i2].tangent = tangent;

		vertices[i0].bitangent = vertices[i1].bitangent = vertices[i2].bitangent = bitangent;
	}

	mMeshes[0].emplace_back(vertices, indices);
	mMeshes.at(0).at(0).uploadToGPU();

	std::vector<Texture> textures;

	if (diffuseTexture) {
		textures.emplace_back(0, ALBEDO,diffuseTexture);
	}

	if (specularTexture) {
		textures.emplace_back(0, SPECULAR, specularTexture);
	}

	if (normalTexture) {
		textures.emplace_back(0, NORMAL,  normalTexture);
	}

	if (heightTexture) {
		textures.emplace_back(0, HEIGHT, heightTexture);
	}

	uint32_t flags{0};
	if (unlit) {
		flags |= UNLIT;
	} else {
		flags |= CASTSHADOW;
	}

	flags |= OPAQUE | TWOSIDED;

	if (color != glm::vec3(0.0f)) {
		flags |= HAS_SOLID_COLOR;
	}
	mMaterial[0] = {.flags = flags, .textureTarget = GL_TEXTURE_2D, .color = color, .textures = textures};
}

Model::Plane::~Plane() = default;

MeshMap& Model::Plane::meshes() {
	return mMeshes;
}

MaterialMap& Model::Plane::material() {
	return mMaterial;
}
