#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs {

inline std::string resolvePath(const std::string_view p) {
    static const auto cwd = std::filesystem::current_path().parent_path();
    return cwd / p;
}

inline std::string readFile(const std::string_view p) {
	std::ifstream file(resolvePath(p));

	if (!file.is_open()) {
		throw std::runtime_error(std::format("Failed to open the file: {}", p));
	}

	std::stringstream ss;
	ss << file.rdbuf();

	return ss.str();
}
}
