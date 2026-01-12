#include <iostream>
#include <ostream>
#include "core/engine.h"
#include "core/camera.h"
#include "core/input.h"
#include "rendering/shader.h"
#include "resourceManager/resourceManager.h"
#include "ECS/registry.h"
#include "ECS/components/bv.hpp"
#include "ECS/components/debug.hpp"
#include "ECS/components/directionalLight.hpp"
#include "ECS/components/material.hpp"
#include "ECS/components/mesh.hpp"
#include "ECS/components/pointLight.hpp"
#include "ECS/components/skybox.hpp"
#include "ECS/components/spotLight.hpp"
#include "ECS/components/transform.hpp"
#include "ECS/components/debug.hpp"
#include "ECS/components/instance.hpp"
#include "math/boundingVolume.h"
#include "rendering/mesh/mesh.h"
#include "rendering/models/cube.h"
#include "rendering/models/cubemap.h"
#include "rendering/models/plane.h"
#include "rendering/models/sphere.h"
#include "rendering/renderContext/renderFlags.hpp"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define STRINGIFY0(s) # s
#define STRINGIFY(s) STRINGIFY0(s)
#define VERSION STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH)

int main() {
	Registry registry;

	try {
		Engine engine;
		engine.init(&registry);

		Models::Cubemap skyboxModel{ResourceManager::instance().getSkyboxTexture()};
		auto skybox = registry.createEntity("Skybox");
		skybox.addComponent<TransformComponent>(
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f));
		skybox.addComponent<SkyboxComponent>();
		skybox.addComponent<MaterialComponent>(skyboxModel.material());
		skybox.addComponent<MeshComponent>(skyboxModel.meshes());
		//
		std::vector<float> transforms = {
			// Instance 0
			-10.8f, 1.3f, -1.2f, 95.0f, 12.0f, 270.0f, 1, 1, 1,
			// Instance 1
			-12.4f, 2.2f, 2.7f, 130.0f, 340.0f, 22.0f, 1, 1, 1,
			// Instance 2
			-13.5f, 3.1f, 0.5f, 250.0f, 80.0f, 190.0f, 1, 1, 1,
			// Instance 3
			-11.3f, 4.9f, -2.6f, 310.0f, 10.0f, 45.0f, 1, 1, 1,
			// Instance 4
			-10.2f, 5.0f, 1.8f, 180.0f, 300.0f, 15.0f, 1, 1, 1,
			// Instance 5
			-10.1f, 6.4f, -0.8f, 60.0f, 45.0f, 310.0f, 1, 1, 1,
			// Instance 6
			-15.9f, 7.5f, -3.1f, 350.0f, 120.0f, 70.0f, 1, 1, 1,
			// Instance 7
			-12.7f, 1.2f, 0.0f, 210.0f, 33.0f, 10.0f, 1, 1, 1,
			// Instance 8
			-10.9f, 4.7f, 2.4f, 145.0f, 200.0f, 310.0f, 1, 1, 1,
			// Instance 9
			-13.9f, 5.1f, -1.7f, 25.0f, 88.0f, 132.0f, 1, 1, 1,
			// Instance 10
			-10.4f, 1.3f, 0.9f, 300.0f, 260.0f, 50.0f, 1, 1, 1,
			// Instance 11
			-11.2f, 5.5f, -0.5f, 110.0f, 170.0f, 256.0f, 1, 1, 1,
			// Instance 12
			-10.1f, 1.0f, -3.0f, 270.0f, 30.0f, 200.0f, 1, 1, 1,
			// Instance 13
			-10.4f, 5.6f, 1.3f, 34.0f, 300.0f, 99.0f, 1, 1, 1,
			// Instance 14
			-13.8f, 5.3f, 0.4f, 190.0f, 210.0f, 140.0f, 1, 1, 1,
			// Instance 15
			-10.0f, 5.2f, -2.1f, 360.0f, 44.0f, 18.0f, 1, 1, 1,
			// Instance 16
			-10.7f, 5.0f, 3.0f, 77.0f, 180.0f, 350.0f, 1, 1, 1,
			// Instance 17
			-11.9f, 6.4f, -0.2f, 150.0f, 330.0f, 65.0f, 1, 1, 1,
			// Instance 18
			-10.0f, 1.2f, 1.6f, 12.0f, 99.0f, 280.0f, 1, 1, 1,
			// Instance 19
			-10.6f, 1.1f, -1.0f, 205.0f, 70.0f, 30.0f, 1, 1, 1,
		};

		// Suzanne
		auto suzanne = registry.createEntity("Suzanne");
		suzanne.addComponent<TransformComponent>(
			glm::vec3(3.2f, 1.1f, 0.0f),
			glm::vec3(1.0f, 45.0f, 23.0f),
			glm::vec3(0.5f));

		ResourceManager::instance().loadModel(suzanne.id(), "Suzanne/glTF/Suzanne.gltf");

		suzanne.addComponent<MeshComponent>(ResourceManager::instance().getMeshes(suzanne.id()));
		suzanne.addComponent<MaterialComponent>(
			ResourceManager::instance().getMaterial(suzanne.id()));

		suzanne.addComponent<BoundingVolumeComponent>(
			std::make_shared<math::AABB>(
				math::generateAABB(*ResourceManager::instance().getMeshes(suzanne.id()))));

		suzanne.addComponent<DebugComponent>();

		//suzanne.addComponent<InstanceComponent>(&transforms);

		//Plane
		// Models::Plane planeModel{"textures/wood.png", "textures/wood_specular.png"};
		// auto plane = registry.createEntity("Plane");
		// plane.addComponent<TransformComponent>(
		// 	glm::vec3(0.0f),
		// 	glm::vec3(0.0f),
		// 	glm::vec3(2.0f));
		//
		// plane.addComponent<MeshComponent>(planeModel.getMeshes());
		// plane.addComponent<MaterialComponent>(planeModel.getMaterial(), 32.0f, 1.0f, Deferred | TwoSided);
		//
		// plane.addComponent<ShaderComponent>(object);
		//
		// plane.addComponent<BoundingVolumeComponent>(
		// 	std::make_shared<math::AABB>(
		// 		math::generateAABB(*planeModel.getMeshes())));
		//
		// plane.addComponent<DebugComponent>();


		// Cube
		// Models::Cube cubeModel{
		// 	"textures/brickwall.jpg",
		// 	"textures/brickwall_specular.jpg",
		// 	"textures/brickwall_normal.jpg"
		// };
		// auto cube = registry.createEntity("Cube");
		// cube.addComponent<TransformComponent>(
		// 	glm::vec3(0.0f, 3.6f, 0.0f),
		// 	glm::vec3(0.0),
		// 	glm::vec3(2.0f));
		//
		// cube.addComponent<MeshComponent>(cubeModel.getMeshes());
		// cube.addComponent<MaterialComponent>(cubeModel.getMaterial(), 32.0f, 1.0f, CastShadow);
		//
		// cube.addComponent<ShaderComponent>(object);
		//
		// cube.addComponent<DebugComponent>();
		//
		// cube.addComponent<BoundingVolumeComponent>(
		// 	std::make_shared<math::AABB>(
		// 		math::generateAABB(*cubeModel.getMeshes())));

		// Helmet
		auto helmet = registry.createEntity("Helmet");
		helmet.addComponent<TransformComponent>(
			glm::vec3(-1.2f, 1.3f, 0.0f),
			glm::vec3(90.0f, 0.0f, 0.0f),
			glm::vec3(1.0f));

		ResourceManager::instance().loadModel(helmet.id(), "DamagedHelmet/glTF/DamagedHelmet.gltf");

		helmet.addComponent<MeshComponent>(ResourceManager::instance().getMeshes(helmet.id()));
		helmet.addComponent<MaterialComponent>(
			ResourceManager::instance().getMaterial(helmet.id()));

		helmet.addComponent<DebugComponent>();

		helmet.addComponent<BoundingVolumeComponent>(
			std::make_shared<math::AABB>(
				math::generateAABB(*ResourceManager::instance().getMeshes(helmet.id()))));

		// auto angel = registry.createEntity("Angel");
		// angel.addComponent<TransformComponent>(
		// 	glm::vec3(-5.2f, 0.0f, -1.0f),
		// 	glm::vec3(-90.0f, 0.0f, 0.0f),
		// 	glm::vec3(2.0f));
		//
		// ResourceManager::instance().loadModel(angel.id(), "cemetery_angel/scene.gltf");
		//
		// angel.addComponent<MeshComponent>(ResourceManager::instance().getMeshes(angel.id()));
		// angel.addComponent<MaterialComponent>(
		// 	ResourceManager::instance().getMaterial(angel.id()));
		//
		// angel.addComponent<DebugComponent>();
		//
		// angel.addComponent<BoundingVolumeComponent>(
		// 	std::make_shared<math::AABB>(
		// 		math::generateAABB(*ResourceManager::instance().getMeshes(angel.id()))));

		// Sponza
		auto sponza = registry.createEntity("Sponza");
		sponza.addComponent<TransformComponent>(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.02f));

		ResourceManager::instance().loadModel(sponza.id(), "Sponza/glTF/Sponza.gltf");

		sponza.addComponent<MeshComponent>(ResourceManager::instance().getMeshes(sponza.id()));
		sponza.addComponent<MaterialComponent>(
			ResourceManager::instance().getMaterial(sponza.id()));

		sponza.addComponent<DebugComponent>();

		sponza.addComponent<BoundingVolumeComponent>(
			std::make_shared<math::AABB>(
				math::generateAABB(*ResourceManager::instance().getMeshes(sponza.id()))));

		//
		Models::Sphere sphereModel{
			glm::vec3(0.0f, 0.0f, 0.0f),
			false,
			"textures/pbr/rusted_iron/albedo.png",
			"textures/pbr/rusted_iron/normal.png",
			"textures/pbr/rusted_iron/metallic.png",
			"textures/pbr/rusted_iron/roughness.png",
			"textures/pbr/rusted_iron/ao.png"
		};
		auto sphere = registry.createEntity("Sphere");
		sphere.addComponent<TransformComponent>(
			glm::vec3(0.0f, 3.6f, 0.0f),
			glm::vec3(0.0),
			glm::vec3(1.0f));

		sphere.addComponent<MeshComponent>(sphereModel.meshes());
		sphere.addComponent<MaterialComponent>(sphereModel.material(), 32.0f, 1.0f);

		sphere.addComponent<DebugComponent>();

		sphere.addComponent<BoundingVolumeComponent>(
			std::make_shared<math::AABB>(
				math::generateAABB(*sphereModel.meshes())));


		auto dirLight = registry.createEntity("Directional Light");
		dirLight.addComponent<DirectionalLightComponent>(
			glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f),
			glm::vec4(0.3f, 0.3f, 0.3f, 0.0f),
			glm::vec4(0.4f, 0.4f, 0.4f, 0.0f),
			glm::vec4(0.5f, 0.5f, 0.5f, 0.0f));
		//
		// auto pointLight = registry.createEntity("Point Light");
		// pointLight.addComponent<TransformComponent>(
		// 	glm::vec3(-3.2f, 5.0f, -2.4f),
		// 	glm::vec3(0.0f, 0.0f, 0.0f),
		// 	glm::vec3(0.2f));
		// //
		// pointLight.addComponent<PointLightComponent>(
		// 	glm::vec4(0.0f),
		// 	glm::vec4(0.3f, 0.3f, 0.3f, 0.0f), // ambient
		// 	glm::vec4(12.0f, 12.2f, 0.0f, 0.0f), // diffuse
		// 	glm::vec4(12.3f, 12.2f, 0.0f, 0.0f), // specular
		// 	glm::vec3(1.0f, 0.14f, 0.07f), // attenuation
		// 	true
		// );
		// //
		// Models::Cube cubeModel{glm::vec3(12.0f, 12.2f, 0.0f), true};
		// pointLight.addComponent<MeshComponent>(cubeModel.meshes());
		// pointLight.addComponent<MaterialComponent>(cubeModel.material(), 32.0f, 1.0f, FORWARD_PASS);
		//
		// pointLight.addComponent<BoundingVolumeComponent>(
		// 	std::make_shared<math::AABB>(
		// 		math::generateAABB(*cubeModel.meshes())));

		// auto pointLight1 = registry.createEntity("Point Light1");
		// pointLight1.addComponent<TransformComponent>(
		// 	glm::vec3(3.2f, 5.0f, -2.4f),
		// 	glm::vec3(0.0f, 0.0f, 0.0f),
		// 	glm::vec3(0.2f));
		// //
		// pointLight1.addComponent<PointLightComponent>(
		// 	glm::vec4(0.0f),
		// 	glm::vec4(0.01f, 0.01f, 0.01f, 0.0f), // ambient
		// 	glm::vec4(0.3f, 0.2f, 0.5f, 0.0f), // diffuse
		// 	glm::vec4(0.3f, 0.2f, 0.5f, 0.0f), // specular
		// 	glm::vec3(1.0f, 0.14f, 0.07f), // attenuation
		// 	false
		// );
		//
		// pointLight1.addComponent<MeshComponent>(cubeModel.getMeshes());
		// pointLight1.addComponent<MaterialComponent>(glm::vec4(1.0f), 32.0f);
		//
		// pointLight1.addComponent<ShaderComponent>(
		// 	std::make_shared<Shader>("models/light.vert", "models/light.frag"));
		//
		// pointLight1.addComponent<BoundingVolumeComponent>(
		// 	std::make_shared<math::AABB>(
		// 		math::generateAABB(*cubeModel.getMeshes())));

		// auto spotLight = registry.createEntity("Spot Light");
		// spotLight.addComponent<TransformComponent>(
		// 	glm::vec3(-3.2f, 5.0f, 0.0f),
		// 	glm::vec3(0.0f, 0.0f, 0.0f),
		// 	glm::vec3(0.2f));
		//
		// spotLight.addComponent<SpotLightComponent>(
		// 	glm::vec4(-3.2f, 5.0f, 0.0f, 0.0f),
		// 	glm::vec4(1.0f, -1.0f, 0.0f, 0.0f),
		// 	glm::vec4(0.1f, 0.1f, 0.1f, 0.0f),
		// 	glm::vec4(1.0f, 1.0f, 1.0f, 0.0f),
		// 	glm::vec4(1.0f, 1.0f, 1.0f, 0.0f),
		// 	glm::vec3(1.0f, 0.045f, 0.0075f),
		// 	true,
		// 	glm::vec4(glm::cos(glm::radians(20.5f)), glm::cos(glm::radians(25.0f)), 0.0f, 0.0f));
		//

		// spotLight.addComponent<MeshComponent>(cubeModel.getMeshes());
		// spotLight.addComponent<MaterialComponent>(cubeModel.getMaterial(), 32.0f, 1.0f, Forward);
		//
		//
		// spotLight.addComponent<BoundingVolumeComponent>(
		// 	std::make_shared<math::AABB>(
		// 		math::generateAABB(*cubeModel.getMeshes())));

		registry.update();

		engine.configure();

		engine.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
