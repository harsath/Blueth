#pragma once
#include "io/IOBuffer.hpp"
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
 * This is the object which the client uses to hold the peer-state object, it
 * might a raw/smart pointer. It's generic, PeerState template, since we leave
 * it upto the user to handle that however they wish. They must use this
 * interface since the internal implementations of the event loop depends on the
 * file descriptor's pointer when an event occures for that particular network
 * socket FD. Since most implementations of event loop uses `union` which
 * restricts us to storing either a pointer or a FD. But since storing FD within
 * a global array is very expensive and waste of the stack memory usage, we'd
 * use the pointer to handle that.
 */
class PeerStateHolder {
      public:
	PeerStateHolder() = default;
	~PeerStateHolder() = default;
	int getFileDescriptor() const noexcept { return fd_; }
	void setFileDescriptor(int fd) noexcept { fd_ = fd; }
	/**
	 * We'll store the remote peer's state of type PeerState which a user
	 * may use to handle the network requests/responses. We'll pass this as
	 * the argument to the network handlers that are registered during the
	 * initilization of the EventLoop implementation. Expected expected type
	 * of PeerState is either a raw or smart pointer(since they might also
	 * pass a unique_ptr which have ownership semantics), we use std::move
	 * to take of that.
	 */
	void *getPeerState() const noexcept { return std::move(peer_state_); }
	void setPeerState(void *peer_state) noexcept {
		peer_state_ = std::move(peer_state);
	}

      private:
	int fd_;
	void *peer_state_;
};

/**
 * The base EventLoopBase is the interface class for the various eventloop
 * abstraction implementations like linux specific epoll or select and so on.
 * Every implementation should support the pure virtual handlers of
 * EventLoopBase which will be used by the users for their code. PeerState is
 * the generic type which a user can specify. It's generally a raw pointer or a
 * smart pointer. It's passed into the callbacks on various events when the file
 * descriptor is ready for read/write/accept events and networking IO
 * operations. PeerState is used to hold user-defined state information like the
 * protocol parser's state and such.
 */
template <typename PeerState> class EventLoopBase {
      public:
	using HandlerCallbackType = std::function<FDStatus(
	    PeerStateHolder *, std::shared_ptr<EventLoopBase<PeerState>>)>;

	/**
	 * Register callbacks for various events like when a file descriptor is
	 * ready for networking IO (read or write) and when a new peer is trying
	 * connect.
	 *
	 * @param callback The callback function (either a functor or a raw
	 * function pointer) which should be invoked when a specific event
	 * happenes.
	 * @param event_type Register the callback for the specific event.
	 */
	virtual void
	registerCallbackForEvent(HandlerCallbackType callback,
				 EventType event_type) noexcept(false) = 0;
	/**
	 * Start the event loop and start processing the requests (assuming all
	 * the callbacks on the events are already set) If the callbacks are not
	 * set at the time of calling this function, an exception of type
	 * std::runtime_error is thrown.
	 */
	virtual void startEventloop() noexcept(false) = 0;
	/**
	 * Write the data in the io_buffer into the remote peer pointed by
	 * peer_fd in a non-blocking way on wire. Returns the amount of bytes
	 * written and raises an exception on failur.
	 *
	 * @param peer_state_holder PeerStateHolder object which contains all
	 * the context information for the write handler to send byte onto the
	 * non-blocking socket
	 * @param io_buffer Bytes to write to the remote peer
	 * @return Total number of bytes written
	 */
	virtual int writeToPeer(
	    PeerStateHolder *peer_state_holder, std::shared_ptr<io::IOBuffer<char>> io_buffer) noexcept(false) = 0;
	/**
	 * Read raw from the wire sent by the remote peer and store it in-place
	 * into the io_buffer. There is zero-copy involved from reading from the
	 * wire and storing it into the io_buffer
	 *
	 * The readFromPeer will try to read number of bytes equal to the total
	 * space available on the buffer. If a user needs to read more content
	 * than the size of the buffer, they must manually resize the IOBuffer
	 * on their end.
	 *
	 * @param peer_state_holder PeerStateHolder object which contains all
	 * the context information needed for reading bytes off the non-blocking
	 * socket of the remote peer
	 * @param io_buffer Pointer to IOBuffer object to store the read bytes
	 * from the wire in-place
	 * @return Total number of bytes read from the remote peer
	 */
	virtual int readFromPeer(
	    PeerStateHolder *peer_state_holder, std::shared_ptr<io::IOBuffer<char>> io_buffer) noexcept(false) = 0;
	virtual ~EventLoopBase() = default;

      protected:
	/**
	 * Get a shared pointer to the current object. It's used when we invoke
	 * the user-defined callbacks on various events like accept/read/write
	 * and we'd pass the shared_ptr reference to the callbacks such such
	 * that they can invoke the writeToPeer and readFromPeer callback
	 * handlers which implements the non-blocking read/write operation on
	 * the underlying peer's socket
	 *
	 * @return Shared pointer to the event loop interface
	 */
	virtual std::shared_ptr<EventLoopBase<PeerState>>
	getSharedPtr() noexcept = 0;
};

struct EpollEventDeleter {
	void operator()(epoll_event *event_ptr) { free(event_ptr); }
};

} // namespace blueth::concurrency
