#pragma once
#include "common.hpp"
#include <functional>
#include <mutex>
#include <shared_mutex>

namespace blueth::concurrency {

// clang-format off
/**
 * This simple interface tightly couples the Mutex with the data which it's
 * protecting. It's mainly used for avoiding deadlocks for a concurrent reads
 * and single update/write patten.
 *
 * The sample usage is as follows,
 *
 * 	*) MutexSynchronize<std::vector<int>> sync_vector;
 *
 * 	Here we created a std::vector<int> which will be protected by a
 * shared_mutex with concurrent reads and a single blocking writes. The access
 * to the underlying std::vector container is *only* through the
 * MutexSynchronize's interface.
 *
 * 	*) sync_vector.withWriteLock([](auto &container){
 * 		container.push_back(10);
 * 		container.pop_back();
 * 		container.resize(10);
 * 	});
 *
 * 	*) sync_vector.withWriteLock([&captured_value, &captured_value_two](auto &container){
 * 		container.push_back(captured_value);
 * 		container.resize(captured_value_two);
 * 	});
 *
 * 	'container' is the reference to the underlying container/data that we
 * are protecting using mutual execution. All the operations within the lambda
 * are guaranteed to be operated on holding the lock. The 'withWriteLock' is
 * only used when we wanted to do update/write operations on the underlying
 * data. It returns 'void', hence the lambda must not contain a return statement
 * which returns a value.
 *
 * 	*) auto value = sync_vector.withReadLock([](const auto &container){
 * 				auto value_at_index_12 = container.at(12);
 * 				return value_at_index_12;
 * 			});
 *
 * 	*) auto size = sync_vector.withReadLock([](const auto &container){
 * 				return container.size();
 * 			});
 *
 * 	The read access can be done in a concurrent manner without any issues,
 * since this interface supports 'concurrent readers' and 'single writer'. When
 * reading, we are guaranteed that the value doesn't get updated in the middle
 * of reading. And same with during write operation, during writing we are
 * guaranteed to write it completely without any concurrent readers accessing
 * the value in the middle.
 *
 */
// clang-format on

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
