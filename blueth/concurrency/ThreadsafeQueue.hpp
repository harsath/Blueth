#pragma once
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <type_traits>

namespace blueth::concurrency {
template <typename T> class thread_safe_queue {
      private:
	std::queue<T> queue_;
	mutable std::mutex mutex_;

      public:
	thread_safe_queue() = default;
	thread_safe_queue(const thread_safe_queue<T> &) = delete;
	thread_safe_queue &operator=(const thread_safe_queue<T> &) = delete;
	thread_safe_queue(thread_safe_queue<T> &&queue_move) noexcept(false) {
		std::lock_guard<std::mutex> op_lock{mutex_};
		if (!queue_.empty()) {
			throw std::logic_error("Moving into a non-empty queue");
		}
		queue_(std::move(queue_move));
	}
	[[nodiscard]] std::size_t size() const noexcept {
		std::lock_guard<std::mutex> op_lock{mutex_};
		return queue_.size();
	}
	[[nodiscard]] std::optional<T> pop() noexcept {
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
} // end namespace blueth::concurrency
