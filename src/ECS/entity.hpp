#pragma once
#include <bitset>

static constexpr size_t MAX_COMPONENTS = 16;
using Signature = std::bitset<MAX_COMPONENTS>;

class Entity {
public:
	Entity() = default;

	explicit Entity(const size_t id, std::string name): mID(id), mName(std::move(name)) {}

	Entity(const Entity& other) = default;

	[[nodiscard]] size_t id() const { return mID; }
	[[nodiscard]] const char* name() const { return mName.c_str(); }

	class Registry* registry{};

	template<typename T, typename ...Args>
	void addComponent(Args&& ...args);

	template<typename T>
	T& getComponent() const;

	template<typename T>
	[[nodiscard]] bool hasComponent() const;

private:
	size_t mID{};
	std::string mName;

public:
	Entity& operator=(const Entity& other) = default;

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
};
