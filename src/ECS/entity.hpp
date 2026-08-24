#pragma once
#include <bitset>

static constexpr size_t MAX_COMPONENTS = 16;
using Signature = std::bitset<MAX_COMPONENTS>;

class Entity {
public:
	Entity() = default;

	explicit Entity(const size_t id, std::string name)
		: mID(id),
		  mName(std::move(name)) {
	}

	Entity(const Entity& other) = default;

	Entity& operator=(const Entity& other) = default;

	[[nodiscard]]
	size_t id() const {
		return mID;
	}

	[[nodiscard]]
	const std::string& name() const {
		return mName;
	}

	class Registry* registry{};

	template<typename T, typename... Args>
	void addComponent(Args&&... args);

	template<typename T>
	T& getComponent() const;

	template<typename T>
	[[nodiscard]]
	bool hasComponent() const;

	template<typename T>
	T* tryGetComponent() const;

	bool operator==(const Entity& other) const {
		return mID == other.mID;
	}

	bool operator!=(const Entity& other) const {
		return mID != other.mID;
	}

	bool operator<(const Entity& other) const {
		return mID < other.mID;
	}

	bool operator>(const Entity& other) const {
		return mID > other.mID;
	}

private:
	size_t mID{};
	std::string mName;
};
