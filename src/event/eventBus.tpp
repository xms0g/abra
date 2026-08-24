#pragma once

template<typename TOwner, typename TEvent>
	void EventBus::subscribeToEvent(TOwner* ownerInstance, void (TOwner::* CallBackFunction)(const TEvent&)) {
	if (!subscribers[typeid(TEvent)].get()) {
		subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
	}

	subscribers[typeid(TEvent)]->emplace_back(
		std::make_unique<EventCallBack<TOwner, TEvent> >(ownerInstance, CallBackFunction));
}

template<typename TEvent, typename... Args>
void EventBus::emitEvent(Args&&... args) {
	auto handlers = subscribers[typeid(TEvent)].get();
	if (handlers) {
		TEvent event{std::forward<Args>(args)...};
		for (auto& handler: *handlers) {
			handler->execute(event);
		}
	}
}
