#include "sceneLoader.h"
#include <fstream>
#include <iostream>
#include "glm/glm.hpp"
#include "nlohmann/json.hpp"
#include "resourceManager.h"
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
#include "../ECS/components/instance.hpp"
#include "../math/boundingVolume.h"
#include "../rendering/material/material.hpp"
#include "../rendering/mesh/mesh.h"
#include "../rendering/models/cube.h"
#include "../rendering/models/plane.h"
#include "../rendering/models/cubemap.h"
#include "../rendering/models/sphere.h"
#include "../rendering/models/terrain.h"
#include "../rendering/renderContext/renderFlags.hpp"

#define TEXTURE_PTR(tex) (!(tex).empty() ? (tex).c_str() : nullptr)

using json = nlohmann::json;

glm::vec3 parseVec3(const json& j) {
	return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
}

void SceneLoader::loadScene(Registry& registry, const std::string& filePath) {
	std::ifstream file(fs::path(filePath));
	if (!file.is_open()) {
		throw std::runtime_error(std::format("Failed to open scene file: {}", filePath));
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

	// Create Entities & Kick off Async Asset Loading
	for (const auto& entityData: sceneJson["entities"]) {
		std::string name = entityData["name"];
		auto entity = registry.createEntity(name);

		if (entityData.contains("model_path")) {
			std::string modelPath = entityData["model_path"];
			rm.asyncLoadModel(entity.id(), modelPath);
		}

		DeferredEntity deferred{entity, entityData["components"]};
		if (entityData.contains("primitive")) {
			deferred.isPrimitive = true;
			deferred.primitiveType = entityData["primitive"];
		}
		deferredEntities.push_back(deferred);
	}

	// Wait for GPU/Disk assets to be ready
	rm.waitForAll();

	// Assemble Components
	for (auto& [entity, comps, isPrimitive, primType]: deferredEntities) {
		if (isPrimitive && (primType == "Cube" || primType == "Plane" || primType == "Terrain")) {
			glm::vec3 color{0.0f};
			bool unlit{false};
			std::string albedoTexture;
			std::string specularTexture;
			std::string normalTexture;
			std::string heightTexture;

			auto matCom = comps["MaterialComponent"];

			if (matCom.contains("color")) {
				color = parseVec3(matCom["color"]);
			}
			if (matCom.contains("unlit")) {
				unlit = matCom["unlit"].get<bool>();
			}

			if (matCom.contains("albedo_texture")) {
				albedoTexture = matCom["albedo_texture"].get<std::string>();
			}

			if (matCom.contains("specular_texture")) {
				specularTexture = matCom["specular_texture"].get<std::string>();
			}

			if (matCom.contains("normal_texture")) {
				normalTexture = matCom["normal_texture"].get<std::string>();
			}

			if (matCom.contains("height_texture")) {
				heightTexture = matCom["height_texture"].get<std::string>();
			}

			auto uploadPrimitives = [&entity]<typename T>(
				const glm::vec3& c,
				bool u,
				const std::string& albedo,
				const std::string& specular,
				const std::string& normal,
				const std::string& height) {
				T model{c, u, TEXTURE_PTR(albedo), TEXTURE_PTR(specular), TEXTURE_PTR(normal), TEXTURE_PTR(height)};

				rm.upload<MeshMap>(entity.id(), model.meshes());
				rm.upload<MaterialMap>(entity.id(), model.material());
			};

			if (primType == "Cube") {
				uploadPrimitives.operator()<Model::Cube>(
					color,
					unlit,
					albedoTexture,
					specularTexture,
					normalTexture,
					heightTexture);
			} else if (primType == "Plane") {
				uploadPrimitives.operator()<Model::Plane>(
					color,
					unlit,
					albedoTexture,
					specularTexture,
					normalTexture,
					heightTexture);
			} else if (primType == "Terrain") {
				uploadPrimitives.operator()<Model::Terrain>(
					color,
					unlit,
					albedoTexture,
					specularTexture,
					normalTexture,
					heightTexture
				);
			}
		} else if (isPrimitive && primType == "Cubemap") {
			auto faces = comps["SkyboxComponent"]["faces"].get<std::vector<std::string> >();

			Model::Cubemap cubemap{faces};

			rm.upload<MeshMap>(entity.id(), cubemap.meshes());
			rm.upload<MaterialMap>(entity.id(), cubemap.material());
		} else if (isPrimitive && primType == "Sphere") {
			glm::vec3 color{0.0f};
			bool unlit{false};
			std::string albedo;
			std::string normal;
			std::string orm;

			auto matCom = comps["MaterialComponent"];
			if (matCom.contains("color")) {
				color = parseVec3(matCom["color"]);
			}
			if (matCom.contains("unlit")) {
				unlit = matCom["unlit"].get<bool>();
			}
			if (matCom.contains("albedo")) {
				albedo = matCom["albedo"].get<std::string>();
			}
			if (matCom.contains("normal")) {
				normal = matCom["normal"].get<std::string>();
			}
			if (matCom.contains("orm")) {
				orm = matCom["orm"].get<std::string>();
			}

			Model::Sphere sphere{
				color,
				unlit,
				TEXTURE_PTR(albedo),
				TEXTURE_PTR(normal),
				TEXTURE_PTR(orm)
			};

			rm.upload<MeshMap>(entity.id(), sphere.meshes());
			rm.upload<MaterialMap>(entity.id(), sphere.material());
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
			entity.addComponent<MeshComponent>(rm.get<MeshMap>(entity.id()));
		}

		// Bounding Volume Component
		if (comps.contains("BoundingVolumeComponent")) {
			auto bvType = comps["BoundingVolumeComponent"]["type"].get<std::string>();
			if (bvType == "AABB") {
				auto meshComp = entity.getComponent<MeshComponent>();

				entity.addComponent<BoundingVolumeComponent>(
					std::make_shared<math::AABB>(math::generateAABB(*meshComp.meshes))
				);
			} else if (bvType == "Dummy") {
				entity.addComponent<BoundingVolumeComponent>(std::make_shared<math::DummyBV>());
			}
		}

		// Instance Component
		if (comps.contains("InstanceComponent")) {
			auto transforms = comps["InstanceComponent"]["transforms"].get<std::vector<float> >();
			rm.upload<decltype(transforms)>(entity.id(), transforms);

			entity.addComponent<InstanceComponent>(rm.get<decltype(transforms)>(entity.id()));
		}

		// Material Component
		if (comps.contains("MaterialComponent")) {
			float heightScale{1.0f};
			if (comps["MaterialComponent"].contains("height_scale")) {
				heightScale = comps["MaterialComponent"]["height_scale"].get<float>();
			}

			uint32_t flags = entity.hasComponent<InstanceComponent>() ? INSTANCED_PASS :
							primType == "Terrain" ? TERRAIN_PASS :
							primType == "Cubemap" ? SKYBOX_PASS : 0;

			entity.addComponent<MaterialComponent>(
				rm.get<MaterialMap>(entity.id()),
				heightScale,
				flags);
		}

		// Debug Component
		if (comps.contains("DebugComponent")) {
			entity.addComponent<DebugComponent>();
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

	rm.uploadModelsToGPU();
}
