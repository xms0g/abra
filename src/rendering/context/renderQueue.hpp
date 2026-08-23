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
	bool empty(std::string_view queueName) const;

	template<typename T>
	RenderQueue<T>& get(std::string_view queueName);

	template<typename T>
	void set(std::string_view queueName);

	template<typename T>
	void emplace(std::string_view queueName, T& group);

private:
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

	std::unordered_map<std::string, std::unique_ptr<IRenderQueue>, StringHash, StringEqual > mQueue;
};

#include "renderQueue.tpp"