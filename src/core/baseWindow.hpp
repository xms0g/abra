#pragma once

template<typename T>
class BaseWindow {
public:
	virtual ~BaseWindow() = default;

	void init(const std::string& title, const int multisamples, const bool fullscreen) {
		initImpl(title, multisamples, fullscreen);
	}

	T* nativeHandle() const {
		return mWindow;
	}

	int32_t width() const {
		return mWidth;
	}

	int32_t height() const {
		return mHeight;
	}

	void clear(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f, const float a = 0.0f) {
		clearImpl(r, g, b, a);
	}

	virtual void swapBuffer() = 0;

protected:
	virtual void initImpl(const std::string& title, int multisamples, bool fullscreen) = 0;

	virtual void clearImpl(float r, float g, float b, float a) = 0;

	T* mWindow{nullptr};
	int mWidth{0}, mHeight{0};
};
