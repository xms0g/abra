#pragma once

template<typename T, typename... Args>
	void Registry::addComponent(const Entity& e, Args&& ... args) {
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
T& Registry::getComponent(const Entity& e) {
	auto* component = tryGetComponent<T>(e);
	assert(component && "Component does not exist");

	return *component;
}

template<typename T>
bool Registry::hasComponent(const Entity& e) {
	const auto componentID = Component<T>::getID();
	return entityComponentSignatures[e.id()].test(componentID);
}

template<typename T>
T* Registry::tryGetComponent(const Entity& e) const {
	const auto componentID = Component<T>::getID();

	const auto& poolIt = componentPools.find(componentID);
	if (poolIt == componentPools.end()) {
		return nullptr;
	}

	auto& pool = static_cast<Pool<T>*>(poolIt->second.get())->data;

	const auto& compIt = pool.find(e.id());
	return compIt != pool.end() ? &compIt->second : nullptr;
}

template<typename T, typename ...Args>
T& Registry::addSystem(Args&& ...args) {
	systems.insert({std::type_index{typeid(T)}, std::make_shared<T>(std::forward<Args>(args)...)});
	return getSystem<T>();

}

template<typename T>
T& Registry::getSystem() const {
	return *std::static_pointer_cast<T>(systems.at(std::type_index{typeid(T)}));
}

template<typename T>
[[nodiscard]]
bool Registry::hasSystem() const {
	return systems.contains(std::type_index{typeid(T)});
}
