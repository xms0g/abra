#pragma once

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

template<typename T>
T* Entity::tryGetComponent() const {
	return registry->tryGetComponent<T>(*this);
}
