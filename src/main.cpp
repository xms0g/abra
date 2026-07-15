#include <iostream>
#include "core/engine.h"
#include "ECS/registry.h"
#include "config/configManager.h"
#include "resource/sceneLoader.h"
#include "resource/resourceManager.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define STRINGIFY0(s) # s
#define STRINGIFY(s) STRINGIFY0(s)
#define VERSION STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH)

int main(int argc, char **argv) {
	try {
		std::string sceneName;
		if (argc > 1) {
			sceneName = argv[1];
		} else {
			throw std::runtime_error("No scene specified");
		}

		Registry registry;
		Engine engine;

		CONFIG_MANAGER_INSTANCE.load("config.toml");
		engine.init(registry);

		SceneLoader::loadScene(registry, sceneName);
		RESOURCE_MANAGER_INSTANCE.createBuffers();

		registry.update();

		engine.configure();
		engine.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
