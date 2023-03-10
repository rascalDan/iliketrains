#pragma once

#include <utility>
class Work;

#if __cpp_lib_semaphore
#	include <deque>
#	include <memory>
#	include <mutex>
#	include <semaphore>
#	include <special_members.hpp>
#	include <thread>
#	include <vector>

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
		requires std::is_base_of_v<Work, T>
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
		requires std::is_base_of_v<Work, T>
	{
		T(std::forward<Params>(params)...).doWork();
	}
};

#endif
