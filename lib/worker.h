#ifndef WORKER_H
#define WORKER_H

#include <utility>

#if __cpp_lib_semaphore
#	include <deque>
#	include <memory>
#	include <mutex>
#	include <semaphore>
#	include <special_members.hpp>
#	include <thread>
#	include <vector>

class Work;

class Worker {
public:
	Worker();
	~Worker();

	NO_COPY(Worker);
	NO_MOVE(Worker);

	using WorkPtr = std::unique_ptr<Work>;

	template<typename T, typename... Params>
	void
	addWork(Params &&... params)
	{
		addWork(std::make_unique<T>(std::forward<Params>(params)...));
	}

	void addWork(WorkPtr w);

private:
	void worker();

	using Threads = std::vector<std::thread>;
	using ToDo = std::deque<WorkPtr>;

	Threads threads;
	ToDo todo;
	std::counting_semaphore<16> todoLen;
	std::mutex todoMutex;
};

#else

class Worker {
public:
	template<typename T, typename... Params>
	void
	addWork(Params &&... params)
	{
		T(std::forward<Params>(params)...).doWork();
	}
};

#endif
#endif
