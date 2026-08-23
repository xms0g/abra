#pragma once

template<typename T>
void GraphicsEncoder::setUniform(std::string_view name, const T& value) {
	assert(mState.pipeline);
	mState.pipeline->setValue(name, value);
}

template<typename T>
void GraphicsEncoder::setUniform(std::string_view name, const T* value, uint32_t count) {
	assert(mState.pipeline);
	mState.pipeline->setValue(name, value, count);
}
