#include "resourceManager.h"
#include <iostream>
#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include "image/stb_image.h"
#include "../config/configManager.h"
#include "../io/filesystem.hpp"
#include "../rendering/material/material.hpp"
#include "../rendering/mesh/mesh.h"
#include "../rendering/mesh/vertex.hpp"
#include "../rendering/texture/texture.h"

ResourceManager& ResourceManager::instance() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::asyncLoadModel(size_t entityID,  std::string modelPath, std::string texturePath) {
	mThreadPool.enqueue([this, entityID, modelPath = std::move(modelPath), texturePath = std::move(texturePath)]() {
		loadModel(entityID, modelPath, texturePath);
	});
}

void ResourceManager::uploadMeshesToGPU() {
	for (auto& [entityID, meshByMaterialID]: mMeshesByEntity) {
		for (auto& [matID, meshes]: meshByMaterialID) {
			for (auto& mesh: meshes) {
				mesh.uploadToGPU();
			}
		}
	}
}

void ResourceManager::uploadMaterialsToGPU() {
	std::unordered_map<std::string, uint32_t> idByPath;
	static std::filesystem::path assetRoot = CONFIG_MANAGER.get<std::string>("path.asset");

	for (auto& [entityID, materials]: mMaterialsByEntity) {
		for (auto& [matID, material]: materials) {
			if (material.textureTarget == TextureTarget::TextureCubeMap) {
				// HDR texture
				if (material.textures.size() == 1) {
					const std::string path = fs::resolvePath(assetRoot / material.textures.front().path);

					Texture texture = Texture::loadHDR(path);

					material.textures.clear();
					material.textures.push_back(std::move(texture));
				} else {
					// Handle 6 faces-cubemap
					std::vector<std::string> paths;
					paths.reserve(material.textures.size());

					for (auto& texture: material.textures) {
						paths.push_back(fs::resolvePath(assetRoot / texture.path));
					}

					Texture texture = Texture::loadCubemap(paths);
					Texture::generateMipmaps({.id = texture.id, .target = texture.target});

					material.textures.clear();
					material.textures.push_back(std::move(texture));
				}

				continue;
			}

			for (auto& texture: material.textures) {
				if (idByPath.contains(texture.path)) {
					texture.id = idByPath.at(texture.path);
					continue;
				}

				auto p = fs::resolvePath(assetRoot / texture.path);
				texture.id = Texture::load(p, material.flags,
					texture.type == aiTextureType_DIFFUSE || texture.type == aiTextureType_EMISSIVE);

				Texture::generateMipmaps({.id = texture.id, .target = texture.target});

				idByPath.emplace(texture.path, texture.id);
			}
		}
	}
}

void ResourceManager::waitForAll() const {
	mThreadPool.wait();
}

void ResourceManager::loadModel(const size_t entityID, std::string_view modelPath, std::string_view texturePath) {
	// read file via ASSIMP
	Assimp::Importer importer;
	const std::filesystem::path assetRoot = CONFIG_MANAGER.get<std::string>("path.asset");

	const std::string path = fs::resolvePath(assetRoot / modelPath);
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
	MaterialLoadContext mlCtx{.textureDir = texturePath.data()};

	processMeshes(scene->mRootNode, scene, meshesByMatID, mlCtx);
	processMaterials(scene, mlCtx);

	{
		std::lock_guard<std::mutex> lock(mResourceMutex);
		mMeshesByEntity.emplace(entityID, std::move(meshesByMatID));
		mMaterialsByEntity.emplace(entityID, std::move(mlCtx.materials));
	}
}

void ResourceManager::processMeshes(
	const aiNode* node,
	const aiScene* scene,
	MeshMap& meshesByMatID,
	MaterialLoadContext& materialLoadCtx) {
	// process each mesh located at the current node
	for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
		// the node object only contains mIndices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
		Mesh mesh = processMesh(aMesh);

		meshesByMatID[aMesh->mMaterialIndex].emplace_back(std::move(mesh));
		materialLoadCtx.materialsToLoad.emplace(aMesh->mMaterialIndex);
	}

	// after we've processed all of the mMeshes (if any) we then recursively process each of the children nodes
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		processMeshes(node->mChildren[i], scene, meshesByMatID, materialLoadCtx);
	}
}

Mesh ResourceManager::processMesh(aiMesh* mesh) {
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
	return {vertices, indices};
}

void ResourceManager::processMaterials(const aiScene* scene, MaterialLoadContext& materialLoadCtx) {
	// process materials
	for (const auto& matID: materialLoadCtx.materialsToLoad) {
		const aiMaterial* material = scene->mMaterials[matID];

		for (const auto& type: textureTypes) {
			TextureLoadRequest req{.mat = material, .type = type, .materialID = matID};
			loadMaterialTextures(req, materialLoadCtx);
		}
	}
}

void ResourceManager::loadMaterialTextures(const TextureLoadRequest& req, MaterialLoadContext& materialLoadCtx) {
	if (!materialLoadCtx.materials.contains(req.materialID)) {
		uint32_t flags{0};
		flags |= CASTSHADOW;

		if (int twoSided{0};
			req.mat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
			flags |= twoSided != 0 ? TWOSIDED : 0;
		}

		if (float matPBR{0.0f};
			req.mat->Get(AI_MATKEY_METALLIC_FACTOR, matPBR) == AI_SUCCESS ||
			req.mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, matPBR) == AI_SUCCESS) {
			flags |= (matPBR > 0.0f) ? PBR : 0;
		}

		// Only supports glTF
		float alphaCutoff{0.0f};

		if (aiString alphaMode;
			req.mat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
			if (std::strcmp(alphaMode.C_Str(), "OPAQUE") == 0) {
				if (!(flags & PBR)) {
					flags |= OPAQUE;
				}
			} else if (std::strcmp(alphaMode.C_Str(), "MASK") == 0) {
				if (flags & PBR) {
					flags |= ALPHACUTOFF;
				} else {
					flags |= OPAQUE | ALPHACUTOFF;
				}
				req.mat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
			} else if (std::strcmp(alphaMode.C_Str(), "BLEND") == 0) {
				flags |= BLEND;
			}
		}

		Material material;
		material.flags = flags;
		material.textureTarget = TextureTarget::Texture2D;
		material.alphaCutoff = alphaCutoff;
		materialLoadCtx.materials[req.materialID] = std::move(material);
	}

	auto& material = materialLoadCtx.materials[req.materialID];

	for (uint32_t i = 0; i < req.mat->GetTextureCount(req.type); ++i) {
		aiString str;

		req.mat->GetTexture(req.type, i, &str);
		std::string path = materialLoadCtx.textureDir + str.C_Str();

		switch (req.type) {
			case aiTextureType_HEIGHT:
				material.flags |= HAS_HEIGHT_MAP;
				break;
			case aiTextureType_EMISSIVE:
				material.flags |= HAS_EMISSIVE_MAP;
				break;
			case aiTextureType_UNKNOWN:
				materialLoadCtx.roughMetalPath = path;
				break;
			case aiTextureType_LIGHTMAP: {
				if (materialLoadCtx.roughMetalPath == path) {
					material.flags |= HAS_ORM;
					return;
				}
				material.flags |= HAS_AO_MAP;
				break;
			}
		}

		material.textures.emplace_back(0, static_cast<uint32_t>(req.type), TextureTarget::Texture2D, std::move(path));
	}
}
