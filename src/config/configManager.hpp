#pragma once
#include <string>
#include <any>
#include <unordered_map>

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

	struct StringHash {
		using is_transparent = void;

		size_t operator()(const std::string_view value) const noexcept {
			return std::hash<std::string_view>{}(value);
		}

	};

	struct StringEqual {
		using is_transparent = void;

		bool operator()(const std::string_view lhs, const std::string_view rhs) const noexcept {
			return lhs == rhs;
		}
	};

	std::unordered_map<std::string, std::any, StringHash, StringEqual> mConfig;
};

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
