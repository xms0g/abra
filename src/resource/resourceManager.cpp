#include "resourceManager.h"
#include <iostream>
#include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
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
#include "../rendering/mesh/vertexArray.h"
#include "../rendering/graphicsEncoder.h"
#include "../rendering/graphicsPipeline.h"

ResourceManager& ResourceManager::instance() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::createBuffers() {
	if (!mBuffers.contains("envMap"))
		return;

	createIrradianceMap();
	createPrefilterMap();
	createBrdfLUT();
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
			if (material.textureTarget == TextureTarget::TextureCubeMap) {
				// Handle HDR to Cubemap
				if (material.textures.size() == 1) {
					const std::string path = fs::path(
						CONFIG_MANAGER_INSTANCE.get<std::string>("path.asset") + material.textures.front().path);

					uint32_t id = createEnvMap(path);

					material.textures.clear();
					material.textures.emplace_back(id, 0, TextureTarget::TextureCubeMap, "");
				} else {
					// Handle 6 faces-cubemap
					std::vector<std::string> paths;
					paths.reserve(material.textures.size());

					for (auto& [id, type, target, path]: material.textures) {
						paths.push_back(fs::path(CONFIG_MANAGER_INSTANCE.get<std::string>("path.asset") + path));
					}

					material.textures.clear();
					material.textures.emplace_back(Texture::loadCubemap(paths), 0, TextureTarget::TextureCubeMap, "");
				}

				continue;
			}

			for (auto& [id, type, target, path]: material.textures) {
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

		material.textures.emplace_back(0, static_cast<uint32_t>(req.type), TextureTarget::Texture2D, std::move(path));
	}
}

uint32_t ResourceManager::createEnvMap(const std::string& path) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Lequal,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("pbr/cubemap.vert", "pbr/equirectangularToCube.frag"),
		.samplers = {
			{.name = "equirectangularMap", .slot = 0}
		},
		.uniforms = {}
	};

	auto pipeline = GraphicsPipeline{info};
	auto encoder = GraphicsEncoder{};

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

	const Texture hdrTexture = Texture::loadHDR(path);

	// convert HDR equirectangular environment map to cubemap equivalent
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.bindTexture({.id = hdrTexture.id, .target = hdrTexture.target}, 0);

	encoder.bindFrameBuffer(*envMapBuffer);

	for (int32_t i = 0; i < FACES; ++i) {
		encoder.attachFramebufferTexture(envMapBuffer->texture(), Attachment::Color0, 0, i);
		encoder.setUniform("view", mCaptureViews[i]);

		encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

		encoder.drawIndexed({
			.vao = cubeMesh.vao().id(),
			.vertexCount = 0,
			.indexCount = cubeMesh.indices().size()
		});
	}

	encoder.unbindFrameBuffer();
	encoder.bindTexture(envMapBuffer->texture(), 0);
	envMapBuffer->generateMipmaps();

	return envMapBuffer->texture().id;
}

void ResourceManager::createIrradianceMap() {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("pbr/cubemap.vert", "pbr/irradianceConv.frag"),
		.samplers = {
			{.name = "environmentMap", .slot = 0}
		},
		.uniforms = {}
	};

	auto pipeline = GraphicsPipeline{info};
	auto encoder = GraphicsEncoder{};

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

	// solve diffuse integral by convolution to create an irradiance (cube)map.
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.bindTexture(envMapBuffer->texture(), 0);

	encoder.bindFrameBuffer(*irradianceMapBuffer);

	for (int32_t i = 0; i < FACES; ++i) {
		encoder.attachFramebufferTexture(irradianceMapBuffer->texture(), Attachment::Color0, 0, i);
		encoder.setUniform("view", mCaptureViews[i]);

		encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

		encoder.drawIndexed({
			.vao = cubeMesh.vao().id(),
			.vertexCount = 0,
			.indexCount = cubeMesh.indices().size()
		});
	}

	encoder.unbindFrameBuffer();
}

void ResourceManager::createPrefilterMap() {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("pbr/cubemap.vert", "pbr/prefilter.frag"),
		.samplers = {
			{.name = "environmentMap", .slot = 0}
		},
		.uniforms = {}
	};

	auto pipeline = GraphicsPipeline{info};
	auto encoder = GraphicsEncoder{};

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

	// run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	encoder.bindPipeline(pipeline);
	encoder.setUniform("projection", mCaptureProjection);
	encoder.setUniform("resolution",
	                   static_cast<float>(CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.envMap.size")));

	encoder.bindTexture(envMapBuffer->texture(), 0);

	encoder.bindFrameBuffer(*prefilterMapBuffer);

	constexpr uint32_t mipLevels = 5;
	for (int32_t i = 0; i < mipLevels; ++i) {
		const int32_t mipSize = static_cast<int32_t>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.size") * std::pow(0.5, i));
		prefilterMapBuffer->resizeRenderBuffer(mipSize, mipSize);

		const float roughness = static_cast<float>(i) / static_cast<float>(mipLevels - 1);
		encoder.setUniform("roughness", roughness);


		for (int32_t j = 0; j < FACES; ++j) {
			encoder.attachFramebufferTexture(prefilterMapBuffer->texture(), Attachment::Color0, i, j);
			encoder.setUniform("view", mCaptureViews[j]);

			encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);
			encoder.drawIndexed({
				.vao = cubeMesh.vao().id(),
				.vertexCount = 0,
				.indexCount = cubeMesh.indices().size()
			});
		}
	}

	encoder.unbindFrameBuffer();
}

void ResourceManager::createBrdfLUT() {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("pbr/brdfLUT.vert", "pbr/brdfLUT.frag"),
		.samplers = {
				{.name = "environmentMap", .slot = 0}
		},
		.uniforms = {}
	};

	auto pipeline = GraphicsPipeline{info};
	auto encoder = GraphicsEncoder{};

	auto brdfLUT = std::make_unique<FrameBuffer>(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.size"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.size"));
	brdfLUT->withTextureFP(GL_RG)
			.checkStatus();

	mBuffers.emplace("brdfLUT", std::move(brdfLUT));

	const auto brdfLUTBuffer = get<BaseFrameBuffer>("brdfLUT");

	const Model::Quad quad;
	// generate a 2D LUT from the BRDF equations used.
	encoder.bindPipeline(pipeline);
	encoder.bindFrameBuffer(*brdfLUTBuffer);
	encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);
	encoder.draw({
		.vao = quad.vao(),
		.vertexCount = 6,
		.indexCount = 0
	});

	encoder.unbindFrameBuffer();
}
