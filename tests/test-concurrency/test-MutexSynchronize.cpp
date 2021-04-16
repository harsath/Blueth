#include "concurrency/MutexSynchronize.hpp"
#include <gtest/gtest.h>

TEST(MutexSynchronizeTest, TestOneSingleThreaded) {
	using namespace blueth;
	struct One {
		std::size_t counter{};
		std::size_t one{};
		One(std::size_t one) : one{one} {}
	};
	concurrency::MutexSynchronize<One> sync(One{1});
	std::size_t value_one = 10;
	sync.withWriteLock([&value_one](auto &state) { state.counter += value_one; });
	std::size_t read_value_one =
	    sync.withReadLock([](const auto &state) { return state.counter; });
	ASSERT_EQ(read_value_one, 10);
}
