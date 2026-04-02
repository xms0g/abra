#include "resourceManager.h"
#include <iostream>
#include <assimp/GltfMaterial.h>
#include "image/stb_image.h"
#include "../rendering/texture/texture.h"
#include "../rendering/mesh/mesh.h"
#include "../io/filesystem.hpp"
#include "../config/config.hpp"
#include "../rendering/material/material.hpp"

ResourceManager& ResourceManager::instance() {
	static ResourceManager instance;
	return instance;
}

[[nodiscard]] MeshMap* ResourceManager::getMeshes(const size_t entityID) {
	return &mMeshesByEntity.at(entityID);
}

[[nodiscard]] const MaterialMap* ResourceManager::getMaterial(const size_t entityID) const {
	return &mMaterialsByEntity.at(entityID);
}

std::span<const char* const> ResourceManager::getSkyboxTexture() const {
	return skyboxFaces;
}

void ResourceManager::asyncLoadModel(size_t entityID, const char* file) {
	mThreadPool.enqueue([this, entityID, file]() {
		loadModel(entityID, std::string(file).c_str());
	});
}

void ResourceManager::uploadModelsToGPU() {
	for (auto& [entityID, meshByMaterial]: mMeshesByEntity) {
		for (auto& [matID, meshes]: meshByMaterial) {
			for (auto& mesh: meshes) {
				mesh.uploadToGPU();
			}
		}
	}

	std::unordered_set<std::string> texturesLoaded;
	for (auto& [entityID, materials]: mMaterialsByEntity) {
		for (auto& [matID, material]: materials) {
			for (auto& texture: material.textures) {
				if (texturesLoaded.contains(texture.path))
					continue;

				texture.id = texture::load(texture.path.c_str(), material.flag);
				texturesLoaded.emplace(texture.path);
			}
		}
	}
}

void ResourceManager::waitForAll() const {
	mThreadPool.wait();
}

void ResourceManager::loadModel(const size_t entityID, const char* file) {
	// read file via ASSIMP
	Assimp::Importer importer;

	const std::string path = fs::path(ASSET_DIR + file);
	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	// process ASSIMP's root node recursively
	MeshMap meshesByMatID;
	processMeshes(scene->mRootNode, scene, meshesByMatID);

	MaterialLoadContext mlCtx{.baseDir = path.substr(0, path.find_last_of('/')).append("/")};
	processMaterials(scene, mlCtx);

	{
		std::lock_guard<std::mutex> lock(mResourceMutex);
		mMeshesByEntity.emplace(entityID, meshesByMatID);
		mMaterialsByEntity.emplace(entityID, mlCtx.materials);
	}
}

void ResourceManager::processMeshes(const aiNode* node, const aiScene* scene, MeshMap& meshesByMatID) {
	// process each mesh located at the current node
	for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
		// the node object only contains mIndices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
		const Mesh mesh = processMesh(aMesh);

		meshesByMatID[aMesh->mMaterialIndex].push_back(mesh);
	}

	// after we've processed all of the mMeshes (if any) we then recursively process each of the children nodes
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		processMeshes(node->mChildren[i], scene, meshesByMatID);
	}
}

Mesh ResourceManager::processMesh(aiMesh* mesh) const {
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// walk through each of the mesh's mVertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex{};
		glm::vec3 vector;
		// we declare a placeholder std::vector since assimp uses its own std::vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		if (mesh->HasPositions()) {
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;
		}
		// normals
		if (mesh->HasNormals()) {
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;
		}
		// texture coordinates
		if (mesh->HasTextureCoords(0)) {
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texcoord = vec;
			// tangent
			if (mesh->HasTangentsAndBitangents()) {
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.tangent = vector;
				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.bitangent = vector;
			}
		} else
			vertex.texcoord = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex mIndices.
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		// retrieve all mIndices of the face and store them in the mIndices std::vector
		for (uint32_t j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);
	}

	// return a mesh object created from the extracted mesh data
	return Mesh{vertices, indices};
}

void ResourceManager::processMaterials(const aiScene* scene, MaterialLoadContext& ctx) const {
	// process materials
	for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
		const aiMaterial* material = scene->mMaterials[i];

		for (const auto& [type, name]: textureBindings) {
			TextureLoadRequest req{
				.mat = material,
				.type = type,
				.typeName = name,
				.materialID = i
			};
			loadMaterialTextures(req, ctx);
		}
	}
}

void ResourceManager::loadMaterialTextures(const TextureLoadRequest& req, MaterialLoadContext& ctx) const {
	for (uint32_t i = 0; i < req.mat->GetTextureCount(req.type); ++i) {
		aiString str;
		req.mat->GetTexture(req.type, i, &str);
		std::string path = ctx.baseDir + str.C_Str();
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		if (ctx.texturesLoaded.contains(path))
			continue;

		// store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate mTextures.
		ctx.texturesLoaded.emplace(path);

		if (ctx.materials.contains(req.materialID)) {
			ctx.materials[req.materialID].textures.emplace_back(0, req.type, req.typeName, std::move(path));
		} else {
			uint32_t flag{0};
			int twoSided{0};
			float matPBR{0.0f};

			if (req.mat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
				flag |= twoSided != 0 ? TWOSIDED : 0;
			}

			if (req.mat->Get(AI_MATKEY_METALLIC_FACTOR, matPBR) == AI_SUCCESS ||
			    req.mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, matPBR) == AI_SUCCESS) {
				flag |= (matPBR > 0.0f) ? PBR : 0;
			}

			// Only supports GLTF
			float alphaCutoff{0.0f};
			aiString alphaMode;

			if (req.mat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
				if (std::strcmp(alphaMode.C_Str(), "OPAQUE") == 0) {
					flag |= OPAQUE | CASTSHADOW;
				} else if (std::strcmp(alphaMode.C_Str(), "MASK") == 0) {
					flag |= CUTOUT | CASTSHADOW;
					req.mat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
				} else if (std::strcmp(alphaMode.C_Str(), "BLEND") == 0) {
					flag |= BLEND;
				}
			}

			std::vector<Texture> textures;
			textures.emplace_back(0, req.type, req.typeName, std::move(path));
			ctx.materials[req.materialID] = {flag, glm::vec3(), alphaCutoff, std::move(textures)};
		}
	}
}
