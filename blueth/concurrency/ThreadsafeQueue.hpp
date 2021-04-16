#pragma once
#include "common.hpp"
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <type_traits>

namespace blueth::concurrency {
template <typename T> class ThreadSafeQueue {
      private:
	std::queue<T> queue_;
	mutable std::mutex mutex_;

      public:
	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue<T> &) = delete;
	ThreadSafeQueue &operator=(const ThreadSafeQueue<T> &) = delete;
	ThreadSafeQueue(ThreadSafeQueue<T> &&queue_move) noexcept(false) {
		std::lock_guard<std::mutex> op_lock{mutex_};
		if (!queue_.empty()) {
			throw std::logic_error("Moving into a non-empty queue");
		}
		queue_(std::move(queue_move));
	}
	BLUETH_NODISCARD std::size_t size() const noexcept {
		std::lock_guard<std::mutex> op_lock{mutex_};
		return queue_.size();
	}
	BLUETH_NODISCARD std::optional<T> pop() noexcept {
		std::lock_guard<std::mutex> op_lock{mutex_};
		if (queue_.empty()) { return std::nullopt; }
		T returner{std::move(queue_.front())};
		queue_.pop();
		return returner;
	}
	void push(const T &data) noexcept {
		std::lock_guard<std::mutex> op_lock{mutex_};
		queue_.push(data);
	}
	void push(T &&data) noexcept {
		std::lock_guard<std::mutex> op_lock{mutex_};
		queue_.emplace(std::move(data));
	}
};
} // namespace blueth::concurrency
