#pragma once
#include "MutexSynchronize.hpp"
#include "ThreadsafeQueue.hpp"
#include "common.hpp"
#include <array>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace blueth::concurrency {

template <typename Callable> class ThreadPoolExecutor {
	using ThreadPoolType = std::vector<std::thread>;

      public:
	/**
	 * Constructor for the ThreadPoolExecutor object
	 *
	 * We create the std::thread objects in a constant array so that we
	 * spin-up the threads equal to the Thread-Pool's size during
	 * construction. Future implementation will be provided with an
	 * implementation where we maintain a Min-Pool-Size and Max-Pool-Size so
	 * when the server is 'hot', and the 'Callable' work is increasing and
	 * bypasses Min-Pool-Size, we create new std::thread objects until
	 * Max-Pool-Size. But the new onces created during the 'hot' period aka
	 * the threads above the size of the Min-Pool-Size will have an internal
	 * timeout field which will make the extra threads beyond Min size to
	 * .join()/die. But we make sure that we maintain the Min-Pool-Size
	 * always on the ThreadPoolExecutor's internal pool for the
	 * future/current 'Callable' work
	 *
	 * @param pool_size ThreadPoolExecutor's initial pool size
	 */
	ThreadPoolExecutor(std::size_t pool_size);
	/**
	 * Submit a work to the ThreadPoolExecutor
	 *
	 * We submit a 'Universal Reference' type of a 'Callable' object, it may
	 * either be an object with operator()() overload or a std::function, a
	 * lambda or a humble function pointer. Internally, we invoke the
	 * 'Callable' of type void<void> aka, a 'Callable' which takes nothing
	 * and returns void. A typical implementation will construct a 'Work'
	 * class with operator()() implementation and we construct the object
	 * with right arguments on the constructor and pass the object onto the
	 * ThreadPoolExecutor for the invocation of the function on the
	 * thread-pool.
	 *
	 * @param callableFunction A object/std::function/lambda/function
	 * pointer
	 */
	void submit(Callable &&callableFunction) noexcept;
	/**
	 * Query the current pool size
	 *
	 * @return Returns 'pool_size_'
	 */
	std::size_t getPoolSize() const noexcept;
	/**
	 * Query the number of 'Callable' objects within the
	 * ThreadPoolConcurrentQueue
	 *
	 * @return Number of active work queued up for execution.
	 */
	std::size_t getTaskCount() const noexcept;
	/**
	 * Query the number of threads currently working not in the idel
	 * stage/waiting for work in the empty queue.
	 *
	 * @return Get number of 'hot' threads on the Thread-Pool
	 */
	std::size_t getActiveThreads() const noexcept;
	/**
	 * Check if the ThreadPoolExecutor has been shutdown
	 *
	 * @return Boolean flag indicating the shutdown.
	 */
	bool isShutdown() const noexcept;
	/**
	 * Shutdown the ThreadPoolExecutor. No new 'Work' will be processed on
	 * the Thread-Pool but the already existing work will be processed. It's
	 * a graceful shutdown. In the future implementation we will maintain a
	 * Time-Out where if threads doesn't pop and execute it's 'Work' in the
	 * specified time, we just forcefully shutdown the Pool.
	 */
	void shutdown() noexcept;
	~ThreadPoolExecutor();

      private:
	// void poolAndExecute();
	std::size_t pool_size_;
	std::size_t active_threads_{};
	bool shutdown_{false};
	ThreadPoolType thread_pool_;
	std::condition_variable worker_cv_;
	std::queue<Callable> work_queue_;
	std::mutex queue_mutex_;
};

template <typename Callable>
ThreadPoolExecutor<Callable>::ThreadPoolExecutor(std::size_t pool_size)
    : pool_size_{pool_size} {
	for (std::size_t i{}; i < pool_size_; ++i) {
		thread_pool_.emplace_back([this]() {
			for (;;) {
				std::optional<Callable> work;
				{
					std::unique_lock<std::mutex> lock(
					    queue_mutex_);
					worker_cv_.wait(lock, [this]() {
						return (shutdown_ ||
							!work_queue_.empty());
					});
					if (shutdown_ && work_queue_.empty())
						return;
					work = std::move(work_queue_.front());
					work_queue_.pop();
				}
				(*work)();
			}
		});
	}
}

template <typename Callable>
std::size_t ThreadPoolExecutor<Callable>::getActiveThreads() const noexcept {
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		return active_threads_;
	}
}

template <typename Callable>
std::size_t ThreadPoolExecutor<Callable>::getTaskCount() const noexcept {
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		return work_queue_.size();
	}
}

template <typename Callable>
void ThreadPoolExecutor<Callable>::submit(
    Callable &&callableFunction) noexcept {
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		work_queue_.emplace(std::forward<Callable>(callableFunction));
	}
	worker_cv_.notify_one();
}

template <typename Callable>
bool ThreadPoolExecutor<Callable>::isShutdown() const noexcept {
	return shutdown_;
}

template <typename Callable>
void ThreadPoolExecutor<Callable>::shutdown() noexcept {
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		shutdown_ = true;
	}
	worker_cv_.notify_all();
}

template <typename Callable>
ThreadPoolExecutor<Callable>::~ThreadPoolExecutor() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		shutdown_ = true;
	}
	worker_cv_.notify_all();
	for (std::thread &worker : thread_pool_) worker.join();
}

} // namespace blueth::concurrency
