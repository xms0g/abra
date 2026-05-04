#pragma once
#include "../../ECS/system.hpp"

class EventBus;

class GuiSystem final : public System {
public:
	GuiSystem();

	~GuiSystem() override;

	void update(float dt);

	void render(EventBus& eventBus) const;

private:
	void updateFpsCounter(float dt);

	// Frame
	double mPreviousSeconds{0.0};
	double mCurrentSeconds{0.0};
	uint32_t mFPS{0};
	uint32_t mCurrentFrameCount{0};
};
