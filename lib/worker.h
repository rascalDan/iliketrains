#pragma once

#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <semaphore>
#include <special_members.h>
#include <thread>
#include <utility>
#include <vector>

class Worker {
public:
	class WorkItem {
	protected:
		WorkItem(Worker * worker) : worker {worker} { }

		virtual ~WorkItem() = default;
		NO_MOVE(WorkItem);
		NO_COPY(WorkItem);

		void
		assist() const
		{
			worker->assist();
		}

		Worker * worker;

	public:
		virtual void doWork() = 0;
	};

	template<typename T> class WorkItemT : public WorkItem {
	public:
		T
		get()
		{
			using namespace std::chrono_literals;
			while (future.wait_for(0s) == std::future_status::timeout) {
				assist();
			}
			return future.get();
		}

	protected:
		WorkItemT(Worker * worker) : WorkItem {worker} { }

		std::promise<T> promise;
		std::future<T> future {promise.get_future()};
		friend Worker;
	};

	template<typename... Params>
	static auto
	addWork(Params &&... params)
	{
		return instance.addWorkImpl(std::forward<Params>(params)...);
	}

	template<typename T> using WorkPtrT = std::shared_ptr<WorkItemT<T>>;

private:
	template<typename T, typename... Params> class WorkItemTImpl : public WorkItemT<T> {
	public:
		WorkItemTImpl(Worker * worker, Params &&... params) :
			WorkItemT<T> {worker}, params {std::forward<Params>(params)...}
		{
		}

	private:
		void
		doWork() override
		{
			try {
				if constexpr (std::is_void_v<T>) {
					std::apply(std::invoke<Params &...>, params);
					WorkItemT<T>::promise.set_value();
				}
				else {
					WorkItemT<T>::promise.set_value(std::apply(std::invoke<Params &...>, params));
				}
			}
			catch (...) {
				WorkItemT<T>::promise.set_exception(std::current_exception());
			}
		}

		std::tuple<Params...> params;
	};

	Worker();
	~Worker();

	NO_COPY(Worker);
	NO_MOVE(Worker);

	using WorkPtr = std::shared_ptr<WorkItem>;

	template<typename... Params>
	auto
	addWorkImpl(Params &&... params)
	{
		using T = decltype(std::invoke(std::forward<Params>(params)...));
		auto work = std::make_shared<WorkItemTImpl<T, Params...>>(this, std::forward<Params>(params)...);
		addWorkPtr(work);
		return work;
	}

	void addWorkPtr(WorkPtr w);
	void worker();
	void assist();

	using Threads = std::vector<std::jthread>;
	using ToDo = std::deque<WorkPtr>;

	ToDo todo;
	std::counting_semaphore<16> todoLen;
	std::mutex todoMutex;
	Threads threads;

	static Worker instance;
};
