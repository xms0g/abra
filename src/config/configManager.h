#pragma once
#include <string>
#include <any>
#include <unordered_map>

class ConfigManager {
public:
	ConfigManager(const ConfigManager&) = delete;

	ConfigManager& operator=(const ConfigManager&) = delete;

	static ConfigManager& instance();

	template<typename T>
	T& get(const std::string& key);

	template<typename T>
	void set(const std::string& key, T&& value);

	void load(const std::string& filepath);

private:
	explicit ConfigManager() = default;

	~ConfigManager() = default;

	std::unordered_map<std::string, std::any> mConfig;
};

template<typename T>
T& ConfigManager::get(const std::string& key) {
	return std::any_cast<T&>(mConfig[key]);
}

template<typename T>
void ConfigManager::set(const std::string& key, T&& value) {
	mConfig[key] = std::forward<T>(value);
}
