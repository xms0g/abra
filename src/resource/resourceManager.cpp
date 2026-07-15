#include "resourceManager.h"
#include <iostream>
#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include "glad/glad.h"
#include "image/stb_image.h"
#include "../config/configManager.h"
#include "../io/filesystem.hpp"
#include "../rendering/shader.h"
#include "../rendering/models/cube.h"
#include "../rendering/models/quad.h"
#include "../rendering/material/material.hpp"
#include "../rendering/mesh/mesh.h"
#include "../rendering/mesh/vertex.hpp"
#include "../rendering/texture/texture.h"
#include "../rendering/buffers/frameBuffer.h"
#include "../rendering/renderCommand.h"
#include "../rendering/mesh/vertexArray.h"

ResourceManager& ResourceManager::instance() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::createBuffers() {
	if (!mBuffers.contains("envMap"))
		return;

	glDisable(GL_CULL_FACE);
	createIrradianceMap();
	createPrefilterMap();
	createBrdfLUT();
	glEnable(GL_CULL_FACE);
}

std::unordered_map<std::string, std::unique_ptr<Shader> >& ResourceManager::getShaders() {
	return mShaders;
}

void ResourceManager::asyncLoadModel(size_t entityID, const std::string& file) {
	mThreadPool.enqueue([this, entityID, file]() {
		loadModel(entityID, file);
	});
}

void ResourceManager::uploadModelsToGPU() {
	for (auto& [entityID, meshByMaterialID]: mMeshesByEntity) {
		for (auto& [matID, meshes]: meshByMaterialID) {
			for (auto& mesh: meshes) {
				mesh.uploadToGPU();
			}
		}
	}

	std::unordered_map<std::string, uint32_t> idByPath;

	for (auto& [entityID, materials]: mMaterialsByEntity) {
		for (auto& [matID, material]: materials) {
			if (material.textureTarget == GL_TEXTURE_CUBE_MAP) {
				// Handle HDR to Cubemap
				if (material.textures.size() == 1) {
					const std::string path = fs::path(CONFIG_MANAGER_INSTANCE.get<std::string>("path.asset") + material.textures.front().path);

					uint32_t id = createEnvMap(path);

					material.textures.clear();
					material.textures.emplace_back(id, 0, "");
				} else {
					// Handle 6 faces-cubemap
					std::vector<std::string> paths;
					paths.reserve(material.textures.size());

					for (auto& [id, type, path]: material.textures) {
						paths.push_back(fs::path(CONFIG_MANAGER_INSTANCE.get<std::string>("path.asset") + path));
					}

					material.textures.clear();
					material.textures.emplace_back(Texture::loadCubemap(paths), 0, "");
				}

				continue;
			}

			for (auto& [id, type, path]: material.textures) {
				if (idByPath.contains(path)) {
					id = idByPath.at(path);
					continue;
				}

				id = Texture::load(
					fs::path(CONFIG_MANAGER_INSTANCE.get<std::string>("path.asset") + path),
					material.flags,
					type == aiTextureType_DIFFUSE || type == aiTextureType_EMISSIVE);

				idByPath.emplace(path, id);
			}
		}
	}
}

void ResourceManager::waitForAll() const {
	mThreadPool.wait();
}

ResourceManager::ResourceManager() {
	createShaders();
}

void ResourceManager::createShaders() {
	// Terrain
	mShaders.emplace(
		"terrain",
		std::make_unique<Shader>("models/terrain.vert","models/terrain.frag","","models/terrain.tcs","models/terrain.tes"));
	// Deferred
	mShaders.emplace("gBuffer", std::make_unique<Shader>("deferred/gbuffer.vert", "deferred/gbuffer.frag"));
	mShaders.emplace("deferredLighting", std::make_unique<Shader>("models/quad.vert", "deferred/lighting.frag"));
	// SSAO
	mShaders.emplace("ssao", std::make_unique<Shader>("models/quad.vert", "ssao.frag"));
	mShaders.emplace("ssaoBlur", std::make_unique<Shader>("models/quad.vert", "ssaoBlur.frag"));
	// Forward
	mShaders.emplace("opaque", std::make_unique<Shader>("object.vert", "opaque.frag"));
	mShaders.emplace("blend", std::make_unique<Shader>("object.vert", "blend.frag"));
	mShaders.emplace("unlit", std::make_unique<Shader>("unlit.vert", "unlit.frag"));
	// Instanced
	mShaders.emplace("opaqueInstanced", std::make_unique<Shader>("instanced.vert", "opaque.frag"));
	mShaders.emplace("blendInstanced", std::make_unique<Shader>("instanced.vert", "blend.frag"));
	// Skybox
	mShaders.emplace("skybox", std::make_unique<Shader>("skybox.vert", "skybox.frag"));
	// Depth
	mShaders.emplace("depth", std::make_unique<Shader>("depth/depth.vert", "depth/depth.frag"));
	mShaders.emplace(
		"depthCubemap",
		std::make_unique<Shader>("depth/depthCubemap.vert", "depth/depthCubemap.frag","depth/depthCubemap.geom"));
	// Debug
	mShaders.emplace(
		"debugNormal",
		std::make_unique<Shader>("debug/normal.vert", "debug/normal.frag", "debug/normal.geom"));
	mShaders.emplace(
		"debugWireframe",
		std::make_unique<Shader>("debug/wireframe.vert", "debug/wireframe.frag", "debug/wireframe.geom"));
	// PBR
	mShaders.emplace(
		"equirectangularToCube",
		std::make_unique<Shader>("pbr/cubemap.vert", "pbr/equirectangularToCube.frag"));
	mShaders.emplace("irradianceConv", std::make_unique<Shader>("pbr/cubemap.vert", "pbr/irradianceConv.frag"));
	mShaders.emplace("prefilter", std::make_unique<Shader>("pbr/cubemap.vert", "pbr/prefilter.frag"));
	mShaders.emplace("brdfLUT", std::make_unique<Shader>("pbr/brdfLUT.vert", "pbr/brdfLUT.frag"));
	// PostFX
	mShaders.emplace(
		"bloomBF",
		std::make_unique<Shader>("models/quad.vert", "post-processing/bloom/brightFilter.frag"));
	mShaders.emplace("bloomBlur", std::make_unique<Shader>("models/quad.vert", "post-processing/bloom/blur.frag"));
	mShaders.emplace(
		"bloomCombine",
		std::make_unique<Shader>("models/quad.vert", "post-processing/bloom/combine.frag"));
	mShaders.emplace("toneMapping", std::make_unique<Shader>("models/quad.vert", "post-processing/toneMapping.frag"));
	mShaders.emplace("grayscale", std::make_unique<Shader>("models/quad.vert", "post-processing/grayscale.frag"));
	mShaders.emplace("sepia", std::make_unique<Shader>("models/quad.vert", "post-processing/sepia.frag"));
	mShaders.emplace("kernel", std::make_unique<Shader>("models/quad.vert", "post-processing/kernel.frag"));
	mShaders.emplace("ca", std::make_unique<Shader>("models/quad.vert", "post-processing/ca.frag"));
	mShaders.emplace("gamma", std::make_unique<Shader>("models/quad.vert", "post-processing/gamma.frag"));
	mShaders.emplace("fxaa", std::make_unique<Shader>("models/quad.vert", "post-processing/fxaa.frag"));
}

void ResourceManager::loadModel(const size_t entityID, const std::string& file) {
	// read file via ASSIMP
	Assimp::Importer importer;

	const std::string path = fs::path(CONFIG_MANAGER_INSTANCE.get<std::string>("path.asset") + file);
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
	MaterialLoadContext mlCtx{.baseDir = file.substr(0, file.find_last_of('/')).append("/")};

	processMeshes(scene->mRootNode, scene, meshesByMatID, mlCtx);
	processMaterials(scene, mlCtx);

	{
		std::lock_guard<std::mutex> lock(mResourceMutex);
		mMeshesByEntity.emplace(entityID, std::move(meshesByMatID));
		mMaterialsByEntity.emplace(entityID, mlCtx.materials);
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

		for (const auto& type: textureBindings) {
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

		materialLoadCtx.materials[req.materialID] = {
				.flags = flags,
				.textureTarget = GL_TEXTURE_2D,
				.alphaCutoff = alphaCutoff,
				.shader = flags & OPAQUE ? RESOURCE_MANAGER_INSTANCE.get<Shader>("opaque") :
				          flags & BLEND ? RESOURCE_MANAGER_INSTANCE.get<Shader>("blend") : nullptr
			};
	}

	auto& material = materialLoadCtx.materials[req.materialID];

	for (uint32_t i = 0; i < req.mat->GetTextureCount(req.type); ++i) {
		aiString str;

		req.mat->GetTexture(req.type, i, &str);
		std::string path = materialLoadCtx.baseDir + str.C_Str();

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

		material.textures.emplace_back(0, req.type, std::move(path));
	}
}

uint32_t ResourceManager::createEnvMap(const std::string& path) {
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	auto envMap = std::make_unique<CubemapBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.envMap.size"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.envMap.size"),
		true);
	envMap->checkStatus();

	mBuffers.emplace("envMap", std::move(envMap));

	const auto& envMapBuffer = mBuffers.at("envMap");

	Model::Cube cube;
	cube.meshes().at(0).front().uploadToGPU();
	const auto& cubeMesh = cube.meshes().at(0).front();

	const auto equirectangularToCube = get<Shader>("equirectangularToCube");
	const Texture hdrTexture = Texture::loadHDR(path);

	// convert HDR equirectangular environment map to cubemap equivalent
	equirectangularToCube->activate();
	equirectangularToCube->setInt("equirectangularMap", 0);
	equirectangularToCube->setMat4("projection", mCaptureProjection);

	hdrTexture.bind(0);

	envMapBuffer->bind();
	for (uint32_t i = 0; i < FACES; ++i) {
		dynamic_cast<CubemapBuffer*>(envMapBuffer.get())->bindFace(i);
		equirectangularToCube->setMat4("view", mCaptureViews[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommand::drawMesh(cubeMesh.vao().id(), cubeMesh.vertices().size(), cubeMesh.indices().size());
	}

	envMapBuffer->unbind();

	envMapBuffer->bindTexture(0);
	envMapBuffer->generateMipmaps();

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	return envMapBuffer->texture();
}

void ResourceManager::createIrradianceMap() {
	auto irradianceMap = std::make_unique<CubemapBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.size"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.size"));
	irradianceMap->checkStatus();

	mBuffers.emplace("irradianceMap", std::move(irradianceMap));

	const auto irradianceMapBuffer = get<BaseFrameBuffer>("irradianceMap");
	const auto envMapBuffer = get<BaseFrameBuffer>("envMap");

	Model::Cube cube;
	cube.meshes().at(0).front().uploadToGPU();
	const auto& cubeMesh = cube.meshes().at(0).front();

	const auto irradianceConv = get<Shader>("irradianceConv");
	// solve diffuse integral by convolution to create an irradiance (cube)map.
	irradianceConv->activate();
	irradianceConv->setInt("environmentMap", 0);
	irradianceConv->setMat4("projection", mCaptureProjection);

	envMapBuffer->bindTexture(0);

	irradianceMapBuffer->bind();
	for (uint32_t i = 0; i < FACES; ++i) {
		dynamic_cast<const CubemapBuffer*>(irradianceMapBuffer)->bindFace(i);
		irradianceConv->setMat4("view", mCaptureViews[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCommand::drawMesh(cubeMesh.vao().id(), cubeMesh.vertices().size(), cubeMesh.indices().size());
	}

	irradianceMapBuffer->unbind();
}

void ResourceManager::createPrefilterMap() {
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	auto prefilterMap = std::make_unique<CubemapBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.size"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.size"),
		true,
		true);
	prefilterMap->checkStatus();

	mBuffers.emplace("prefilterMap", std::move(prefilterMap));

	const auto prefilterMapBuffer = get<BaseFrameBuffer>("prefilterMap");
	const auto envMapBuffer = get<BaseFrameBuffer>("envMap");

	Model::Cube cube;
	cube.meshes().at(0).front().uploadToGPU();
	const auto& cubeMesh = cube.meshes().at(0).front();

	const auto prefilter = get<Shader>("prefilter");
	// run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	prefilter->activate();
	prefilter->setInt("environmentMap", 0);
	prefilter->setMat4("projection", mCaptureProjection);
	prefilter->setFloat("resolution",
		static_cast<float>(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.envMap.size")));

	envMapBuffer->bindTexture(0);
	prefilterMapBuffer->bind();

	constexpr uint32_t mipLevels = 5;
	for (int32_t i = 0; i < mipLevels; ++i) {
		const int32_t mipSize = static_cast<int32_t>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.size") * std::pow(0.5, i));

		prefilterMapBuffer->resizeRenderBuffer(mipSize, mipSize);

		const float roughness = static_cast<float>(i) / static_cast<float>(mipLevels - 1);
		prefilter->setFloat("roughness", roughness);

		for (uint32_t j = 0; j < FACES; ++j) {
			dynamic_cast<const CubemapBuffer*>(prefilterMapBuffer)->bindFace(j, i);
			prefilter->setMat4("view", mCaptureViews[j]);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCommand::drawMesh(cubeMesh.vao().id(), cubeMesh.vertices().size(), cubeMesh.indices().size());
		}
	}

	prefilterMapBuffer->unbind();
}

void ResourceManager::createBrdfLUT() {
	auto brdfLUT = std::make_unique<FrameBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.size"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.size"));
	brdfLUT->withTextureFP(GL_RG)
			.checkStatus();

	mBuffers.emplace("brdfLUT", std::move(brdfLUT));

	const auto brdfLUTBuffer = get<BaseFrameBuffer>("brdfLUT");

	const Model::SingleQuad quad;
	// generate a 2D LUT from the BRDF equations used.
	const auto brdfLUTShader = get<Shader>("brdfLUT");
	brdfLUTShader->activate();

	brdfLUTBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderCommand::drawQuad(quad.vao());

	brdfLUTBuffer->unbind();
}
