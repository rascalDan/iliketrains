#include "worker.h"
#include <algorithm>
#include <iterator>
#include <mutex>

Worker Worker::instance;

Worker::Worker() : todoLen {0}
{
	std::generate_n(std::back_inserter(threads), std::thread::hardware_concurrency(), [this]() {
		return std::jthread {&Worker::worker, this};
	});
	if constexpr (requires { pthread_setname_np(std::declval<std::jthread>().native_handle(), ""); }) {
		for (auto & thread : threads) {
			pthread_setname_np(thread.native_handle(), "ilt-worker");
		}
	}
}

Worker::~Worker()
{
	todoLen.release(std::thread::hardware_concurrency());
}

void
Worker::addWorkPtr(WorkPtr j)
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

void
Worker::assist()
{
	auto job = [this]() {
		using namespace std::chrono_literals;
		if (todoLen.try_acquire_for(100us)) {
			if (std::lock_guard<std::mutex> lck {todoMutex}; todo.size()) {
				WorkPtr x = std::move(todo.front());
				if (x) {
					todo.pop_front();
				}
				return x;
			}
		}
		return WorkPtr {};
	};
	if (auto j = job()) {
		j->doWork();
	}
}
