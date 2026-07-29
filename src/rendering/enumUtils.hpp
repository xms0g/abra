#pragma once
#include <type_traits>

template<typename T>
constexpr auto toUnderlying(T e) {
	return static_cast<std::underlying_type_t<T>>(e);
}

#define GLu(type) \
constexpr uint32_t toGLu(const type v) { \
	return toUnderlying(v); \
}

#define GLi(type) \
constexpr int32_t toGLi(const type v) { \
	return toUnderlying(v); \
}
