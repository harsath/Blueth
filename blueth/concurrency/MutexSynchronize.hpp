#pragma once
#include "common.hpp"
#include <functional>
#include <mutex>
#include <shared_mutex>

namespace blueth::concurrency {

/**
 * This simple interface tightly couples the Mutex with it's data which it's
 * protecting. It's mainly used for avoiding deadlocks for a concurrent reads
 * and single writes.
 */

template <typename TypeName> class MutexSynchronize {
      public:
	MutexSynchronize(const TypeName &data) : data_{data_} {}
	MutexSynchronize(TypeName &&data) : data_{std::move(data)} {}
	template <typename WriteHandlerType>
	void withWriteLock(WriteHandlerType &&writeFunction) {
		std::lock_guard<std::shared_mutex> write_lock{data_mutex_};
		writeFunction(data_);
	}
	template <typename ReadHandlerType>
	decltype(auto) withReadLock(ReadHandlerType &&readFunction) {
		std::shared_lock<std::shared_mutex> read_lock{data_mutex_};
		return readFunction(data_);
	}
	std::shared_mutex getSharedMutex() noexcept { return data_mutex_; }

      private:
	TypeName data_;
	std::shared_mutex data_mutex_;
};

} // namespace blueth::concurrency
