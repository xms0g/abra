#pragma once
#include <string>

struct Config;
class Registry;

namespace  SceneLoader {
void loadScene(Registry& registry, const std::string& filePath);
}
