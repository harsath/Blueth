#pragma once
#include "ThreadPoolConcurrentQueue.hpp"
#include "common.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace blueth::concurrency {

template <typename Callable> class ThreadPoolExecutor {
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

      private:
	void poolAndExecute();
	std::size_t pool_size_;
};

} // namespace blueth::concurrency
