#include "worker.h"
#include "work.h"
#include <algorithm>
#include <iterator>
#include <mutex>

Worker Worker::instance;

Worker::Worker() : todoLen {0}
{
	std::generate_n(std::back_inserter(threads), std::thread::hardware_concurrency(), [this]() {
		return std::jthread {&Worker::worker, this};
	});
}

Worker::~Worker()
{
	todoLen.release(std::thread::hardware_concurrency());
}

void
Worker::addWork(WorkPtr j)
{
	std::lock_guard<std::mutex> lck {todoMutex};
	todoLen.release();
	todo.emplace_back(std::move(j));
}

void
Worker::worker()
{
	auto job = [this]() {
		todoLen.acquire();
		std::lock_guard<std::mutex> lck {todoMutex};
		if (todo.size()) {
			WorkPtr x = std::move(todo.front());
			todo.pop_front();
			return x;
		}
		return WorkPtr {};
	};
	while (auto j = job()) {
		j->doWork();
	}
}
