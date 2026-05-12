#pragma once

struct GuiLightEvent;
class EventBus;
struct RenderContext;
struct GuiTransformEvent;
struct GuiDebugEvent;

class SyncStateSystem {
public:
	~SyncStateSystem();

	void configure(const RenderContext& ctx, EventBus& eventBus);

private:
	void onDebugUpdate(const GuiDebugEvent& event);

	void onTransformUpdate(const GuiTransformEvent& event);

	void onLightUpdate(const GuiLightEvent& event);

	EventBus* mEventBus{nullptr};
	const RenderContext* mCtx{nullptr};
};
