#pragma once
#include <vector>
#include "component.hpp"
#include "entity.hpp"

class System {
public:
	System() = default;

	virtual ~System() = default;

	void addEntityToSystem(const Entity& entity) {
		entities.emplace_back(entity);
	}

	[[nodiscard]]
	const std::vector<Entity>& getSystemEntities() const {
		return entities;
	}

	[[nodiscard]]
	const Signature& getComponentSignature() const {
		return componentSignature;
	}

	[[nodiscard]]
	bool useOrLogic() const {
		return mUseOrLogic;
	}

	template<typename T>
	void RequireComponent(bool orLogic = false);

private:
	bool mUseOrLogic{};
	Signature componentSignature;
	std::vector<Entity> entities;
};

template<typename T>
void System::RequireComponent(const bool orLogic) {
	const auto componentID = Component<T>::getID();
	componentSignature.set(componentID);
	mUseOrLogic = orLogic;
}