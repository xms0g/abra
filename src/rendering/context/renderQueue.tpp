#pragma once

template<typename T>
RenderQueue<T>& QueueRegistry::get(const std::string_view queueName) {
	const auto it = mQueue.find(queueName);

	if (it == mQueue.end())
		throw std::out_of_range("Render queue not found");

	return static_cast<RenderQueue<T>&>(*it->second);
}

template<typename T>
void QueueRegistry::set(const std::string_view queueName) {
	mQueue.emplace(queueName, std::make_unique<RenderQueue<T> >());
}

template<typename T>
void QueueRegistry::emplace(const std::string_view queueName, T& group) {
	try {
		get<T>(queueName).push_back(group);
	} catch (std::out_of_range&) {

	}
}

