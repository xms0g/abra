#pragma once
#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <set>
#include "entity.hpp"
#include "component.hpp"
#include "system.hpp"

class Registry {
public:
	Entity createEntity(const std::string& name);

	void update();

	template<typename T, typename... Args>
	void addComponent(const Entity& e, Args&& ... args) {
		const auto componentID = Component<T>::getID();

		if (!componentPools[componentID]) {
			componentPools[componentID] = std::make_unique<Pool<T>>();
		}

		auto& pool = componentPools[componentID];

		T newComponent(std::forward<Args>(args)...);

		static_cast<Pool<T>*>(pool.get())->data[e.id()] = newComponent;
		entityComponentSignatures[e.id()].set(componentID);
	}

	template<typename T>
	T& getComponent(const Entity& e) {
		const auto componentID = Component<T>::getID();
		auto& pool = componentPools[componentID];

		return static_cast<Pool<T>*>(pool.get())->data[e.id()];
	}

	template<typename T>
	bool hasComponent(const Entity& e) {
		const auto componentID = Component<T>::getID();
		return entityComponentSignatures[e.id()].test(componentID);
	}

	template<typename T, typename ...Args>
	void addSystem(Args&& ...args) {
		systems.insert({std::type_index{typeid(T)}, std::make_shared<T>(std::forward<Args>(args)...)});

	}

	template<typename T>
	T& getSystem() const {
		return *std::static_pointer_cast<T>(systems.at(std::type_index{typeid(T)}));
	}

	template<typename T>
	[[nodiscard]]
	bool hasSystem() const {
		return systems.contains(std::type_index{typeid(T)});
	}

private:
	void addEntityToSystems(const Entity& entity);

	struct IPool {
		virtual ~IPool() = default;
	};

	template<typename T>
	struct Pool final : IPool {
		std::unordered_map<int, T> data;
	};

	size_t numEntities{0};
	std::set<Entity> entitiesToBeAdded;
	std::vector<Signature> entityComponentSignatures;
	std::unordered_map<int, std::unique_ptr<IPool> > componentPools;
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
};

template<typename T, typename ...Args>
void Entity::addComponent(Args&& ...args) {
	registry->addComponent<T>(*this, std::forward<Args>(args)...);
}

template<typename T>
T& Entity::getComponent() const {
	return registry->getComponent<T>(*this);
}

template<typename T>
[[nodiscard]]
bool Entity::hasComponent() const {
	return registry->hasComponent<T>(*this);
}


