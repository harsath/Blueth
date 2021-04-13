#pragma once
#include "common.hpp"
#include "utils/UnixTime.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace blueth::concurrency {

template <typename Callable> class ThreadPoolConcurrentQueue {
      public:

      private:
	std::queue<Callable> queue_;
	mutable std::mutex mutex_;
	std::condition_variable cv_;	
};

} // namespace blueth::concurrency
