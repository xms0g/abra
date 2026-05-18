#include "sceneLoader.h"
#include <fstream>
#include <iostream>
#include "glm/glm.hpp"
#include "nlohmann/json.hpp"
#include "../io/filesystem.hpp"
#include "../ECS/registry.h"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/bv.hpp"
#include "../ECS/components/directionalLight.hpp"
#include "../ECS/components/pointLight.hpp"
#include "../ECS/components/spotLight.hpp"
#include "../ECS/components/skybox.hpp"
#include "../ECS/components/instance.hpp"
#include "../math/boundingVolume.h"
#include "../resourceManager/resourceManager.h"
#include "../rendering/models/cube.h"
#include "../rendering/models/cubemap.h"

using json = nlohmann::json;

glm::vec3 parseVec3(const json& j) {
	return {j[0].get<float>(), j[1].get<float>(), j[2].get<float>()};
}

bool SceneLoader::loadScene(const char* filePath, Registry& registry) {
	std::ifstream file(fs::path(filePath));
	if (!file.is_open()) {
		std::cerr << "Failed to open scene file: " << filePath << std::endl;
		return false;
	}

	json sceneJson;
	file >> sceneJson;
	file.close();

	struct DeferredEntity {
		Entity entity;
		json componentsJson;
		bool isPrimitive{false};
		std::string primitiveType;
	};

	std::vector<DeferredEntity> deferredEntities;

	// --- PHASE 1: Create Entities & Kick off Async Asset Loading ---
	for (const auto& entityData: sceneJson["entities"]) {
		std::string name = entityData["name"];
		auto entity = registry.createEntity(name);

		if (entityData.contains("model_path")) {
			std::string modelPath = entityData["model_path"];
			ResourceManager::instance().asyncLoadModel(entity.id(), modelPath);
		}

		DeferredEntity deferred{entity, entityData["components"]};
		if (entityData.contains("primitive")) {
			deferred.isPrimitive = true;
			deferred.primitiveType = entityData["primitive"];
		}
		deferredEntities.push_back(deferred);
	}

	// --- PHASE 2: Wait for GPU/Disk assets to be ready ---
	ResourceManager::instance().waitForAll();

	// --- PHASE 3: Assemble Components ---
	for (auto& [entity, comps, isPrimitive, primType]: deferredEntities) {
		// Handle Primitive instantiation if necessary
		std::shared_ptr<Models::Cube> localCube = nullptr;
		if (isPrimitive && primType == "Cube") {
			localCube = std::make_shared<Models::Cube>(glm::vec3(1.0f), true);
			mCubes.push_back(localCube);
		} else if (isPrimitive && primType == "Cubemap") {
			if (comps.contains("SkyboxComponent")) {
				std::vector<std::string> faceStrings;

				auto sc = comps["SkyboxComponent"];
				for (const auto& face: sc["faces"]) {
					faceStrings.push_back(face.get<std::string>());
				}

				mSkybox = std::make_shared<Models::Cubemap>(faceStrings);
			}
		}

		// Transform Component
		if (comps.contains("TransformComponent")) {
			auto tc = comps["TransformComponent"];
			entity.addComponent<TransformComponent>(
				parseVec3(tc["position"]),
				parseVec3(tc["rotation"]),
				parseVec3(tc["scale"])
			);
		}

		// Mesh Component
		if (comps.contains("MeshComponent")) {
			if (isPrimitive && primType == "Cube") {
				entity.addComponent<MeshComponent>(localCube->meshes());
			} else if (isPrimitive && primType == "Cubemap") {
				entity.addComponent<MeshComponent>(mSkybox->meshes());
			} else {
				entity.addComponent<MeshComponent>(ResourceManager::instance().getMeshes(entity.id()));
			}
		}

		// Bounding Volume Component
		if (comps.contains("BoundingVolumeComponent")) {
			auto bvc = comps["BoundingVolumeComponent"];
			if (bvc["type"] == "AABB") {
				auto meshComp = entity.getComponent<MeshComponent>();
				entity.addComponent<BoundingVolumeComponent>(
					std::make_shared<math::AABB>(math::generateAABB(*meshComp.meshes))
				);
			}
		}

		// Material Component
		if (comps.contains("MaterialComponent")) {
			if (isPrimitive && primType == "Cube") {
				entity.addComponent<MaterialComponent>(localCube->material());
			} else if (isPrimitive && primType == "Cubemap") {
				entity.addComponent<MaterialComponent>(mSkybox->material());
			} else {
				entity.addComponent<MaterialComponent>(ResourceManager::instance().getMaterial(entity.id()));
			}
		}

		// Debug Component
		if (comps.contains("DebugComponent")) {
			entity.addComponent<DebugComponent>();
		}

		// Instance Component
		if (comps.contains("InstanceComponent")) {
			auto ic = comps["InstanceComponent"];
			auto transforms = ic["transforms"].get<std::vector<float>>();

			entity.addComponent<InstanceComponent>(ResourceManager::instance().uploadTransforms(entity.id(), transforms));
		}

		// Skybox Component
		if (comps.contains("SkyboxComponent")) {
			entity.addComponent<SkyboxComponent>();
		}

		// Directional Light Component
		if (comps.contains("DirectionalLightComponent")) {
			auto dl = comps["DirectionalLightComponent"];
			entity.addComponent<DirectionalLightComponent>(
				dl["index"].get<unsigned int>(),
				parseVec3(dl["direction"]),
				parseVec3(dl["ambient"]),
				parseVec3(dl["diffuse"]),
				parseVec3(dl["specular"]),
				dl["intensity"].get<float>()
			);
		}

		// Point Light Component
		if (comps.contains("PointLightComponent")) {
			auto pl = comps["PointLightComponent"];
			entity.addComponent<PointLightComponent>(
				pl["index"].get<unsigned int>(),
				parseVec3(pl["position"]),
				parseVec3(pl["ambient"]),
				parseVec3(pl["diffuse"]),
				parseVec3(pl["specular"]),
				pl["constant"].get<float>(),
				pl["linear"].get<float>(),
				pl["quadratic"].get<float>(),
				pl["intensity"].get<float>(),
				pl["cast_shadows"].get<bool>()
			);
		}

		// SpotLight Component
		if (comps.contains("SpotLightComponent")) {
			auto sl = comps["SpotLightComponent"];
			entity.addComponent<SpotLightComponent>(
				sl["index"].get<unsigned int>(),
				parseVec3(sl["position"]),
				parseVec3(sl["direction"]),
				parseVec3(sl["ambient"]),
				parseVec3(sl["diffuse"]),
				parseVec3(sl["specular"]),
				sl["constant"].get<float>(),
				sl["linear"].get<float>(),
				sl["quadratic"].get<float>(),
				glm::cos(glm::radians(sl["inner_cutoff_deg"].get<float>())),
				glm::cos(glm::radians(sl["outer_cutoff_deg"].get<float>())),
				sl["intensity"].get<float>(),
				sl["cast_shadows"].get<bool>()
			);
		}
	}

	ResourceManager::instance().uploadModelsToGPU();

	return true;
}
