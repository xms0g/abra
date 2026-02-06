#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
	explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
	~ThreadPool();

	void enqueue(std::function<void()> job);
	void wait() const;

private:
	void workerLoop();

	std::vector<std::thread> mWorkers;
	std::queue<std::function<void()>> mJobs;

	std::mutex mMutex;
	std::condition_variable mCV;
	std::atomic<bool> mStop{false};
	std::atomic<size_t> mActiveJobs{0};
};
