#pragma once

#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <semaphore>
#include <special_members.hpp>
#include <thread>
#include <utility>
#include <vector>

class Worker {
public:
	class WorkItem {
	public:
		WorkItem() = default;
		virtual ~WorkItem() = default;
		NO_MOVE(WorkItem);
		NO_COPY(WorkItem);

		virtual void doWork() = 0;
	};

	template<typename T> class WorkItemT : public WorkItem {
	public:
		T
		get()
		{
			return future.get();
		}

	protected:
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
		WorkItemTImpl(Params &&... params) : params {std::forward<Params>(params)...} { }

	private:
		void
		doWork() override
		{
			try {
				if constexpr (std::is_void_v<T>) {
					std::apply(
							[](auto &&... p) {
								return std::invoke(p...);
							},
							params);
					WorkItemT<T>::promise.set_value();
				}
				else {
					WorkItemT<T>::promise.set_value(std::apply(
							[](auto &&... p) {
								return std::invoke(p...);
							},
							params));
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
		auto work = std::make_shared<WorkItemTImpl<T, Params...>>(std::forward<Params>(params)...);
		addWorkPtr(work);
		return work;
	}

	void addWorkPtr(WorkPtr w);
	void worker();

	using Threads = std::vector<std::jthread>;
	using ToDo = std::deque<WorkPtr>;

	ToDo todo;
	std::counting_semaphore<16> todoLen;
	std::mutex todoMutex;
	Threads threads;

	static Worker instance;
};
