#include <concurrency/ThreadPoolExecutor.hpp>
#include <functional>
#include <gtest/gtest.h>

struct ExecutorWork {
	int value_;
	ExecutorWork(int value) noexcept : value_{value} {}
	void operator()() { std::cout << "value: " << value_ << std::endl; }
};

TEST(ThreadPoolExecutorTest, TestOne) {
	using namespace blueth;

	concurrency::ThreadPoolExecutor<ExecutorWork> executor(4);
	executor.submit(ExecutorWork{1});
	executor.submit(ExecutorWork{2});
	executor.submit(ExecutorWork{3});
	executor.submit(ExecutorWork{4});
	executor.submit(ExecutorWork{5});
	executor.shutdown();
}
