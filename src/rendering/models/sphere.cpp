#include "sphere.h"
#include "../mesh/mesh.h"
#include "../texture/texture.h"
#include "../material/material.hpp"
#include "../../config/config.hpp"
#include "../../io/filesystem.hpp"

Models::Sphere::Sphere(
	glm::vec3 color,
	bool unlit,
	const char* albedo,
	const char* normal,
	const char* metallicRoughness,
	const char* ao) {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uv;
	std::vector<uint32_t> indices;

	constexpr unsigned int X_SEGMENTS = 64;
	constexpr unsigned int Y_SEGMENTS = 64;
	constexpr float PI = 3.14159265359f;

	for (uint32_t x = 0; x <= X_SEGMENTS; ++x) {
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
			float xSegment = static_cast<float>(x) / static_cast<float>(X_SEGMENTS);
			float ySegment = static_cast<float>(y) / static_cast<float>(Y_SEGMENTS);
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			positions.emplace_back(xPos, yPos, zPos);
			uv.emplace_back(xSegment, ySegment);
			normals.emplace_back(xPos, yPos, zPos);
		}
	}

	for (uint32_t y = 0; y < Y_SEGMENTS; ++y) {
		for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
			unsigned int i0 = y * (X_SEGMENTS + 1) + x;
			unsigned int i1 = (y + 1) * (X_SEGMENTS + 1) + x;
			unsigned int i2 = (y + 1) * (X_SEGMENTS + 1) + x + 1;
			unsigned int i3 = y * (X_SEGMENTS + 1) + x + 1;

			indices.push_back(i0);
			indices.push_back(i1);
			indices.push_back(i2);

			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i3);
		}
	}

	std::vector<float> v;
	for (uint32_t i = 0; i < positions.size(); ++i) {
		v.push_back(positions[i].x);
		v.push_back(positions[i].y);
		v.push_back(positions[i].z);
		if (!normals.empty()) {
			v.push_back(normals[i].x);
			v.push_back(normals[i].y);
			v.push_back(normals[i].z);
		}
		if (!uv.empty()) {
			v.push_back(uv[i].x);
			v.push_back(uv[i].y);
		}
	}

	std::vector<Vertex> vertices;
	for (uint32_t i = 0; i < v.size(); i += 8) {
		Vertex vertex{};
		vertex.position = glm::vec3(v[i], v[i + 1], v[i + 2]);
		vertex.normal = glm::vec3(v[i + 3], v[i + 4], v[i + 5]);
		vertex.texcoord = glm::vec2(v[i + 6], v[i + 7]);

		vertices.emplace_back(vertex);
	}

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
	if (albedo) {
		textures.emplace_back(
			texture::load(fs::path(ASSET_DIR + albedo).c_str(), 1),
			ALBEDO,
			"texture_albedo",
			albedo
		);
	}

	if (normal) {
		textures.emplace_back(
			texture::load(fs::path(ASSET_DIR + normal).c_str(), 1),
			NORMAL,
			"texture_normal",
			normal);
	}

	if (metallicRoughness) {
		textures.emplace_back(
			texture::load(fs::path(ASSET_DIR + metallicRoughness).c_str(), 1),
			METALLIC_ROUGHNESS,
			"texture_metallicRoughness",
			metallicRoughness);
	}

	if (ao) {
		textures.emplace_back(
			texture::load(fs::path(ASSET_DIR + ao).c_str(), 1),
			AO,
			"texture_ao",
			ao);
	}

	uint32_t flag{0};
	if (unlit) {
		flag |= UNLIT;
	} else {
		flag |= CASTSHADOW;
	}

	if (metallicRoughness)
		flag |= PBR;

	mMaterial[0] = {flag, color, 0.0f, textures};
}

Models::Sphere::~Sphere() = default;

MeshMap* Models::Sphere::meshes() {
	return &mMeshes;
}

const MaterialMap* Models::Sphere::material() const {
	return &mMaterial;
}
