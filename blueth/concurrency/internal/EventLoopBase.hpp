#pragma once
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/epoll.h>
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

enum class EventType { WriteEvent, ReadEvent, AcceptEvent };

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

/**
 * A user must inherit this class and override the two functions, because these
 * will be used by our event-dispatch handlers to remove/add a file-descriptor
 * from watch list
 */
template <typename PeerState> class PeerStateHolderBase {
      public:
	/**
	 * Get a File-Descriptor
	 *
	 * We use this getter and setter for the file-descriptor for
	 * PeerState when we need to add/remove an it from the watch-list
	 * @return FD which is for this current peer
	 */
	virtual int getFD() = 0;
	/**
	 * Set a File-Descriptor
	 *
	 * We use this when we first accept a connection and allocate a object
	 * for the peer.
	 *
	 * @param fd File descriptor value to set.
	 */
	virtual void setFD(int fd) = 0;
	/**
	 * Get a PeerState, which is a pointer to the object which stores the
	 * Peer's state.
	 *
	 * This will be used by the user if they need to manage the
	 * pointers/objects which maintains the state
	 * @return Pointer(or smart_pointer) to object which store this peer's
	 * state
	 */
	virtual PeerState *getPeerStatePtr() = 0;
	/**
	 * Set a Peer's state. This is useful when we first allocate the object
	 * itself or if we use a smart_pointer which have ownership symentics so
	 * we need to use the std::move
	 *
	 * @param ptr The pointer to object(This class does not mange the
	 * life-time of that object, unless it's a smart pointer which does when
	 * this class dies.)
	 */
	virtual void setPeerStatePtr(PeerState *ptr) = 0;
};

template <typename PeerState> class EventLoopBase {
      public:
	virtual void registerCallbackForEvent(
	    std::function<FDStatus(PeerStateHolderBase<PeerState> *)>,
	    EventType) noexcept(false) = 0;
	virtual void removePeerFromWatchlist(
	    PeerStateHolderBase<PeerState> *) noexcept(false) = 0;
	virtual void modifyEventForPeer(
	    PeerStateHolderBase<PeerState> *) noexcept(false) = 0;
	virtual void startEventloop() noexcept(false) = 0;
	virtual ~EventLoopBase() = default;
};

struct EpollEventDeleter {
	void operator()(epoll_event *event_ptr) { free(event_ptr); }
};

} // namespace blueth::concurrency
