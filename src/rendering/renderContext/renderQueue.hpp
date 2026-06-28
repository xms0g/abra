#pragma once
#include <any>
#include <vector>
#include <string>
#include <unordered_map>

class RenderQueue {
public:
	RenderQueue() = default;

	template<typename T>
	T& get(const std::string& key);

	template<typename T>
	void set(const std::string& key, T&& queue);

private:
	std::unordered_map<std::string, std::any> mQueue;

};

template<typename T>
T& RenderQueue::get(const std::string& key) {
	return std::any_cast<T&>(mQueue[key]);
}

template<typename T>
void RenderQueue::set(const std::string& key, T&& queue) {
	mQueue[key] = std::forward<T>(queue);
}
