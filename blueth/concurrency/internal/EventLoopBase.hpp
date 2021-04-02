#pragma once
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <utility>

namespace blueth::concurrency {

struct FDStatus {
	bool want_read, want_write;
	constexpr FDStatus(bool want_read, bool want_write) noexcept
	    : want_read{want_read}, want_write{want_write} {}
	constexpr FDStatus(const FDStatus &fd_status) noexcept
	    : want_read{fd_status.want_read}, want_write{fd_status.want_write} {
	}
	constexpr FDStatus &operator=(const FDStatus &fd_status) noexcept {
		want_read = fd_status.want_read;
		want_write = fd_status.want_write;
		return *this;
	}
	constexpr FDStatus(FDStatus &&fd_status) noexcept
	    : want_read{fd_status.want_read}, want_write{fd_status.want_write} {
	}
	constexpr FDStatus() noexcept : want_read{false}, want_write{false} {}
};

static constexpr FDStatus WantRead{true, false};
static constexpr FDStatus WantWrite{false, true};
static constexpr FDStatus WantReadWrite{true, true};
static constexpr FDStatus WantNoReadWrite{false, false};

enum class event_type { on_ready_read, on_ready_write };

static void make_socketnonblocking(int socket_fd) noexcept {
	int flags = ::fcntl(socket_fd, F_GETFL, 0);
	if (flags == -1) {
		std::perror("fcntl");
		::exit(EXIT_FAILURE);
	}
	if (::fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::perror("fcntl");
		::exit(EXIT_FAILURE);
	}
}

template<typename PeerState> class PeerStateBase {
	public:
	  virtual int get_fd() = 0;
	  virtual void set_fd(int fd) = 0;
};

template <typename PeerStatePtr> class EventLoopBase {
      public:
	virtual void
	    register_callback_accept(std::function<FDStatus(PeerStatePtr)>) = 0;
	virtual void
	    register_callback_event(std::function<FDStatus(PeerStatePtr)>,
				    event_type) = 0;
	virtual void remove_callback_event(PeerStatePtr, event_type) = 0;
};

} // namespace blueth::concurrency
