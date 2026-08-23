#pragma once

template<typename T>
void GraphicsPipeline::setValue(std::string_view name, const T& value) {
	mState.shader.setValue(name, value);
}

template<typename T>
void GraphicsPipeline::setValue(std::string_view name, const T* value, uint32_t count) {
	mState.shader.setValue(name, value, count);
}
