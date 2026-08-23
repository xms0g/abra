#include "threadPool.hpp"

ThreadPool::ThreadPool(const size_t threadCount) {
	for (size_t i = 0; i < threadCount; ++i) {
		mWorkers.emplace_back(&ThreadPool::workerLoop, this);
	}
}

ThreadPool::~ThreadPool() {
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStop = true;
		mHasJob.notify_all();
	}

	for (auto& t : mWorkers) {
		if (t.joinable())
			t.join();
	}
}

void ThreadPool::enqueue(std::function<void()> job) {
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mJobs.emplace(std::move(job));
	}

	++mActiveJobs;
	mHasJob.notify_one();
}

void ThreadPool::wait() const {
	while (mActiveJobs > 0) {
		std::this_thread::yield();
	}
}

void ThreadPool::workerLoop() {
	for (;;) {
		std::function<void()> job;

		{
			std::unique_lock<std::mutex> lock(mMutex);
			mHasJob.wait(lock, [&]() {
				return mStop || !mJobs.empty();
			});

			if (mStop && mJobs.empty())
				return;

			job = std::move(mJobs.front());
			mJobs.pop();
		}

		job();
		--mActiveJobs;
	}
}
