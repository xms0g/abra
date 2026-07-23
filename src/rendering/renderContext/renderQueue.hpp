#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class IRenderQueue {
public:
	virtual ~IRenderQueue() = default;

	[[nodiscard]]
	virtual bool isEmpty() const = 0;
};

template<typename T>
class RenderQueue final : public IRenderQueue, public std::vector<T> {
public:
	RenderQueue() = default;

	[[nodiscard]]
	bool isEmpty() const override { return this->empty(); }
};

class QueueRegistry {
public:
	QueueRegistry() = default;

	[[nodiscard]]
	bool empty(const std::string& queueName) const;

	template<typename T>
	RenderQueue<T>& get(const std::string& queueName);

	template<typename T>
	void set(const std::string& queueName);

	template<typename T>
	void emplace(const std::string& queueName, T& value);

private:
	std::unordered_map<std::string, std::unique_ptr<IRenderQueue>> mQueue;
};

inline bool QueueRegistry::empty(const std::string& queueName) const {
	return mQueue.at(queueName)->isEmpty();
}

template<typename T>
RenderQueue<T>& QueueRegistry::get(const std::string& queueName) {
	return static_cast<RenderQueue<T>&>(*mQueue.at(queueName));
}

template<typename T>
void QueueRegistry::set(const std::string& queueName) {
	mQueue.emplace(queueName, std::make_unique<RenderQueue<T>>());
}

template<typename T>
void QueueRegistry::emplace(const std::string& queueName, T& value) {
	get<T>(queueName).push_back(value);
}
