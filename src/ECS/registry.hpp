#pragma once
#include <cassert>
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
	void addComponent(const Entity& e, Args&& ... args);

	template<typename T>
	T& getComponent(const Entity& e);

	template<typename T>
	bool hasComponent(const Entity& e);

	template<typename T>
	T* tryGetComponent(const Entity& e) const;

	template<typename T, typename ...Args>
	T& addSystem(Args&& ...args);

	template<typename T>
	T& getSystem() const;

	template<typename T>
	[[nodiscard]]
	bool hasSystem() const;

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

#include "registry.tpp"
#include "entity.tpp"