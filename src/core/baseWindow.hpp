#pragma once

template<typename T>
class BaseWindow {
public:
	virtual ~BaseWindow() = default;

	T* nativeHandle() const {
		return mWindow;
	}

	void clear(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f, const float a = 0.0f) {
		clearImpl(r, g, b, a);
	}

	virtual void swapBuffer() = 0;

protected:
	virtual void clearImpl(float r, float g, float b, float a) = 0;

	T* mWindow{nullptr};
};
