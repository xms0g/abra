#pragma once

template<typename T>
T& ConfigManager::get(const std::string_view key) {
	const auto it = mConfig.find(key);

	if (it == mConfig.end())
		throw std::runtime_error("Config key not found: " + std::string(key));

	return std::any_cast<T&>(it->second);
}

template<typename T>
void ConfigManager::set(const std::string_view key, T&& value) {
	mConfig[std::string(key)] = std::forward<T>(value);
}
