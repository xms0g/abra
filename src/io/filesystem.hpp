#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs {

inline std::string path(const std::string& p) {
    auto cwd = std::filesystem::current_path().parent_path();
    return cwd.append(p).string();
}

inline std::string readFile(const std::string& p) {
	std::ifstream file(path(p));

	if (!file.is_open()) {
		throw std::runtime_error(std::string("Failed to open the file: ") + p);
	}

	std::stringstream ss;
	ss << file.rdbuf();

	return ss.str();
}
}
