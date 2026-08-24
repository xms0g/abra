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

#include "configManager.tpp"
