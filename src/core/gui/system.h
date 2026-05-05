#pragma once
#include "../../ECS/system.hpp"

struct EntityState;
class EventBus;

class GuiSystem final : public System {
public:
	GuiSystem();

	~GuiSystem() override;

	void update(float dt);

	void configure();

	void render(EventBus& eventBus);

private:
	void updateFpsCounter(float dt);

	// Frame
	double mPreviousSeconds{0.0};
	double mCurrentSeconds{0.0};
	uint32_t mFPS{0};
	uint32_t mCurrentFrameCount{0};

	std::vector<EntityState> mEntityStates;
};
