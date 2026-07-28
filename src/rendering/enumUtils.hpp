#pragma once
#include <type_traits>

template<typename T>
constexpr auto toUnderlying(T e) {
	return static_cast<std::underlying_type_t<T>>(e);
}

#define GL(type) \
constexpr uint32_t toGL(const type v) { \
	return toUnderlying(v); \
}
