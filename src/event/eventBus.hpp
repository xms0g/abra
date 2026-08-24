#pragma once
#include <list>
#include <typeindex>
#include <unordered_map>
#include "events/event.hpp"

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
class EventCallBack final : public IEventCallBack {
public:
	typedef void (TOwner::* CallBackFunction)(const TEvent&);

	EventCallBack(TOwner* ownerInstance, const CallBackFunction callBackFunction)
		: ownerInstance(ownerInstance),
		  callBackFunction(callBackFunction) {
	}

	~EventCallBack() override = default;

private:
	void call(Event& e) override {
		std::invoke(callBackFunction, ownerInstance, static_cast<TEvent&>(e));
	}

	TOwner* ownerInstance;
	CallBackFunction callBackFunction;
};

class EventBus {
public:
	template<typename TOwner, typename TEvent>
	void subscribeToEvent(TOwner* ownerInstance, void (TOwner::* CallBackFunction)(const TEvent&));

	template<typename TEvent, typename... Args>
	void emitEvent(Args&&... args);

private:
	using HandlerList = std::list<std::unique_ptr<IEventCallBack> >;
	std::unordered_map<std::type_index, std::unique_ptr<HandlerList> > subscribers;
};

#include "eventBus.tpp"
