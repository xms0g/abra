#pragma once
#include <string>
#include <any>
#include <unordered_map>
#include "../utils/string.hpp"

#define CONFIG_MANAGER ConfigManager::instance()

class ConfigManager {
public:
	ConfigManager(const ConfigManager&) = delete;

	ConfigManager& operator=(const ConfigManager&) = delete;

	static ConfigManager& instance();

	template<typename T>
	T& get(std::string_view key);

	template<typename T>
	void set(std::string_view key, T&& value);

	void load(std::string_view filepath);

private:
	explicit ConfigManager() = default;

	~ConfigManager() = default;

	std::unordered_map<std::string, std::any, StringHash, StringEqual> mConfig;
};

#include "configManager.tpp"
