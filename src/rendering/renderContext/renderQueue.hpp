#pragma once
#include <any>
#include <vector>
#include <string>
#include <unordered_map>

class RenderQueue {
public:
	RenderQueue() = default;

	template<typename T>
	T& get(const std::string& queueName);

	template<typename T>
	void set(const std::string& queueName, T&& queue);

	template<typename T>
	void emplace(const std::string& queueName, T& value);

private:
	std::unordered_map<std::string, std::any> mQueue;

};

template<typename T>
T& RenderQueue::get(const std::string& queueName) {
	return std::any_cast<T&>(mQueue[queueName]);
}

template<typename T>
void RenderQueue::set(const std::string& queueName, T&& queue) {
	mQueue[queueName] = std::forward<T>(queue);
}

template<typename T>
void RenderQueue::emplace(const std::string& queueName, T& value) {
	get<std::vector<T>>(queueName).push_back(value);
}
