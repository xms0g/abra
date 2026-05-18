#pragma once
#include <vector>
#include <memory>

namespace Models {
class Cubemap;
class Cube;
}

class Registry;

class SceneLoader {
public:
	bool loadScene(const char* filePath, Registry& registry);

private:
	std::shared_ptr<Models::Cubemap> mSkybox;
	std::vector<std::shared_ptr<Models::Cube>> mCubes;
};
