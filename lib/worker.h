#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <semaphore>
#include <special_members.hpp>
#include <thread>
#include <utility>
#include <vector>

class Work;
class Worker {
private:
	Worker();
	~Worker();

	NO_COPY(Worker);
	NO_MOVE(Worker);

	using WorkPtr = std::unique_ptr<Work>;

	template<typename T, typename... Params>
	void
	addWork(Params &&... params)
		requires std::is_base_of_v<Work, T>
	{
		addWork(std::make_unique<T>(std::forward<Params>(params)...));
	}

	void addWork(WorkPtr w);

private:
	void worker();

	using Threads = std::vector<std::jthread>;
	using ToDo = std::deque<WorkPtr>;

	ToDo todo;
	std::counting_semaphore<16> todoLen;
	std::mutex todoMutex;
	Threads threads;

	static Worker instance;
};
