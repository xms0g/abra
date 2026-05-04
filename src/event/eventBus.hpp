#pragma once
#include <list>
#include <typeindex>
#include <map>
#include "event.hpp"

class IEventCallBack {
public:
	virtual ~IEventCallBack() = default;

	void execute(Event& e) {
		call(e);
	}

private:
	virtual void call(Event& e) = 0;
};

template<typename TOwner, typename TEvent>
class EventCallBack : public IEventCallBack {
public:
	typedef void (TOwner::* CallBackFunction)(const TEvent&);

	EventCallBack(TOwner* ownerInstance, const CallBackFunction callBackFunction)
		: ownerInstance(ownerInstance),
		  callBackFunction(callBackFunction) {
	}

	~EventCallBack() override = default;

private:
	TOwner* ownerInstance;
	CallBackFunction callBackFunction;

	void call(Event& e) override {
		std::invoke(callBackFunction, ownerInstance, static_cast<TEvent&>(e));
	}
};

class EventBus {
public:
	template<typename TOwner, typename TEvent>
	void subscribeToEvent(TOwner* ownerInstance, void (TOwner::* CallBackFunction)(const TEvent&)) {
		if (!subscribers[typeid(TEvent)].get()) {
			subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
		}

		subscribers[typeid(TEvent)]->emplace_back(
			std::make_unique<EventCallBack<TOwner, TEvent> >(ownerInstance, CallBackFunction));
	}

	template<typename TEvent, typename... Args>
	void emitEvent(Args&&... args) {
		auto handlers = subscribers[typeid(TEvent)].get();
		if (handlers) {
			TEvent event{std::forward<Args>(args)...};
			for (auto& handler: *handlers) {
				handler->execute(event);
			}
		}
	}

private:
	using HandlerList = std::list<std::unique_ptr<IEventCallBack> >;
	std::map<std::type_index, std::unique_ptr<HandlerList> > subscribers;
};
