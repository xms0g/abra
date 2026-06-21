#include <iostream>
#include "config/configManager.h"
#include "core/engine.h"
#include "ECS/registry.h"
#include "resourceManager/sceneLoader.h"
#include "resourceManager/resourceManager.h"

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
		ConfigManager::instance().load("config.toml");
		engine.init(&registry);

		ResourceManager::instance().createShaders();
		SceneLoader::loadScene(registry, "assets/scenes/scene_Sponza_Spotlight.json");
		ResourceManager::instance().createBuffers();

		registry.update();
		engine.configure();

		engine.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
