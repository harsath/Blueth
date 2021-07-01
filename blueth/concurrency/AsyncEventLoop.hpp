#pragma once
#include "internal/EventLoopBase.hpp"
#include "io/IOBuffer.hpp"
#include "net/Socket.hpp"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#define CAST_TO_PEERSTATEHOLDER_PTR(pointer) ((PeerStateHolder *)(pointer))
#define CAST_TO_VOID_PTR(pointer) ((void *)(pointer))
#define CAST_TO_PEERSTATE_PTR(pointer) ((PeerState *)pointer)

namespace blueth::concurrency {

template <typename PeerState>
class AsyncEpollEventLoop final
    : public EventLoopBase<PeerState>,
      public std::enable_shared_from_this<AsyncEpollEventLoop<PeerState>> {
      public:
	using HandlerCallbackType =
	    typename EventLoopBase<PeerState>::HandlerCallbackType;
	template <typename T1, typename T2, typename T3, typename T4,
		  typename T5>
	static std::shared_ptr<EventLoopBase<PeerState>>
	create(T1 server_address, T2 server_port, T3 event_loop_size,
	       T4 server_backlog, T5 timeout) {
		return std::make_shared<AsyncEpollEventLoop<PeerState>>(
		    std::move(server_address), std::move(server_port),
		    std::move(event_loop_size), std::move(server_backlog),
		    std::move(timeout));
	}
	AsyncEpollEventLoop(std::string server_address,
			    std::uint16_t server_port, size_t num_event_size,
			    int server_backlog, int timeout) noexcept(false);
	void
	registerCallbackForEvent(HandlerCallbackType callback_fn,
				 EventType event_type) noexcept(false) override;
	void startEventloop() noexcept(false) override;
	int writeToPeer(PeerStateHolder *peer_state_holder,
			std::shared_ptr<io::IOBuffer<char>>
			    io_buffer) noexcept(false) override;
	int readFromPeer(PeerStateHolder *peer_state_holder,
			 std::shared_ptr<io::IOBuffer<char>>
			     io_buffer) noexcept(false) override;
	~AsyncEpollEventLoop();

      protected:
	void epollAddToWatchlist(int fd, PeerStateHolder *peer_state,
				 std::uint32_t event) noexcept(false);
	void modifyEventForPeer(int fd, PeerStateHolder *peer_state,
				std::uint32_t event) noexcept(false);
	void
	removePeerFromWatchlist(int fd,
				PeerStateHolder *peer_state) noexcept(false);
	void addPeerToWatchlist(int fd, PeerStateHolder *peer_state,
				std::uint32_t event) noexcept(false);
	std::shared_ptr<EventLoopBase<PeerState>>
	getSharedPtr() noexcept override {
		return this->shared_from_this();
	}

      private:
	void epollErrorHandler_(int return_code, const char *str) const
	    noexcept(false);

      private:
	net::Socket socket_;
	int epoll_fd_;
	int timeout_;
	size_t max_events_supported_;
	bool epoll_setup_done_{false};
	epoll_event *events_;
	HandlerCallbackType on_accept_callback_;
	HandlerCallbackType on_read_callback_;
	HandlerCallbackType on_write_callback_;
};

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::epollAddToWatchlist(
    int fd, PeerStateHolder *peer_state, std::uint32_t event) noexcept(false) {
	epoll_event ev;
	ev.events = event;
	ev.data.ptr = peer_state;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
		std::perror("epoll_ctl");
		throw std::runtime_error{"epoll_ctl"};
	}
}

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::epollErrorHandler_(int return_code,
							const char *str) const
    noexcept(false) {
	if (return_code < 0) {
		std::perror(str);
		throw std::runtime_error{""};
	}
}

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::removePeerFromWatchlist(
    int fd, PeerStateHolder *peer_state) noexcept(false) {
	epoll_event ev; // Kernel version < 2.6.9 compatibility
	ev.events = 0;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) < 0) {
		std::perror("epoll_ctl");
		throw std::runtime_error{"epoll_ctl"};
	}
}

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::addPeerToWatchlist(
    int fd, PeerStateHolder *peer_state, std::uint32_t event) noexcept(false) {
	epoll_event ev;
	ev.events = event;
	ev.data.ptr = CAST_TO_VOID_PTR(peer_state);
	int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
	epollErrorHandler_(ret, "epoll_ctl");
}

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::modifyEventForPeer(
    int fd, PeerStateHolder *peer_state, std::uint32_t event) noexcept(false) {
	epoll_event ev;
	ev.events = event;
	ev.data.ptr = CAST_TO_VOID_PTR(peer_state);
	int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
	epollErrorHandler_(ret, "epoll_ctl");
}

template <typename PeerState>
AsyncEpollEventLoop<PeerState>::AsyncEpollEventLoop(std::string server_address,
						    std::uint16_t server_port,
						    size_t max_events_supported,
						    int server_backlog,
						    int timeout) noexcept(false)
    : socket_{std::move(server_address), server_port, server_backlog,
	      net::Domain::Ipv4, net::SockType::Stream},
      max_events_supported_{max_events_supported}, timeout_{timeout} {

	socket_.makeSocketNonBlocking();
	socket_.bindSock();
	epoll_fd_ = ::epoll_create1(0);
	epollErrorHandler_(epoll_fd_, "epoll_create1");

	PeerStateHolder *peer_state = new PeerStateHolder();
	peer_state->setFileDescriptor(socket_.getFileDescriptor());
	peer_state->setPeerState(new PeerState());
	epollAddToWatchlist(socket_.getFileDescriptor(), peer_state, EPOLLIN);
	events_ =
	    (epoll_event *)calloc(max_events_supported_, sizeof(epoll_event));
	if (events_ == nullptr) {
		std::perror("calloc()");
		throw std::bad_alloc();
	}
	epoll_setup_done_ = true;
}

template <typename PeerState>
AsyncEpollEventLoop<PeerState>::~AsyncEpollEventLoop() {
	::close(epoll_fd_);
	::free(events_);
}

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::registerCallbackForEvent(
    HandlerCallbackType callback, EventType event) noexcept(false) {
	if (event == EventType::ReadEvent) {
		on_write_callback_ = std::move(callback);
	} else if (event == EventType::WriteEvent) {
		on_read_callback_ = std::move(callback);
	} else if (event == EventType::AcceptEvent) {
		on_accept_callback_ = std::move(callback);
	} else {
		throw std::runtime_error(
		    "Invaild argument to callback register function");
	}
}

template <typename PeerState>
int AsyncEpollEventLoop<PeerState>::writeToPeer(
    PeerStateHolder *peer_state_holder,
    std::shared_ptr<io::IOBuffer<char>> io_buffer) noexcept(false) {
	PeerState *peer_state =
	    CAST_TO_PEERSTATE_PTR(peer_state_holder->getPeerState());
	if (!io_buffer)
		throw std::runtime_error{
		    "Invalid IOBuffer passed onto the writeToPeer handler"};
	if (!io_buffer->getDataSize()) return 0;
	std::size_t num_bytes_to_send = io_buffer->getDataSize();
	int send_ret =
	    ::send(peer_state_holder->getFileDescriptor(),
		   io_buffer->getStartOffsetPointer(), num_bytes_to_send, 0);
	if (send_ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		} else {
			std::perror("send()");
			throw std::runtime_error{""};
		}
	}
	io_buffer->modifyStartOffset(num_bytes_to_send);
	return num_bytes_to_send;
}

template <typename PeerState>
int AsyncEpollEventLoop<PeerState>::readFromPeer(
    PeerStateHolder *peer_state_holder,
    std::shared_ptr<io::IOBuffer<char>> io_buffer) noexcept(false) {
	if (!io_buffer)
		throw std::runtime_error{
		    "Invalid IOBuffer passed onto the readFromPeer handler"};
	if (!io_buffer->getAvailableSpace()) return 0;
	int recv_ret = ::recv(peer_state_holder->getFileDescriptor(),
			      io_buffer->getStartOffsetPointer(),
			      io_buffer->getAvailableSpace(), 0);
	if (recv_ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		} else {
			std::perror("recv()");
			throw std::runtime_error{""};
		}
	}
	io_buffer->modifyEndOffset(recv_ret);
	return recv_ret;
}

// clang-format off
template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::startEventloop() noexcept(false) {
	for (;;) {
		int nready =
		    epoll_wait(epoll_fd_, events_, max_events_supported_, timeout_);
		if (!nready) break;
		epollErrorHandler_(nready, "epoll_wait");
		for (int peer_index{}; peer_index < nready; peer_index++) {
			if (CAST_TO_PEERSTATEHOLDER_PTR(events_[peer_index].data.ptr)->getFileDescriptor() ==
			    socket_.getFileDescriptor()) {
				// New incomming connection
				// @@@ Currently we only support IPv4 for the event loop
				sockaddr_in peer_addr;
				socklen_t peer_addr_len = sizeof(peer_addr);
				int client_fd = ::accept(
				    socket_.getFileDescriptor(), (struct sockaddr *)&peer_addr, &peer_addr_len);
				if (client_fd < 0) {
					if (errno == EAGAIN ||
					    errno == EWOULDBLOCK) {
						fprintf(stderr, "accept EAGAIN or EWOULDBLOCK");
						continue;
					} else {
						std::perror("accept");
						throw std::runtime_error{""};
					}
				}
				PeerStateHolder *peer_state =
				    new PeerStateHolder();
				peer_state->setFileDescriptor(client_fd);
				peer_state->setPeerState(new PeerState());
				make_socketnonblocking(client_fd);
				std::shared_ptr<EventLoopBase<PeerState>> ev_loop = this->getSharedPtr();
				FDStatus fd_status =
				    on_accept_callback_(peer_state, ev_loop);
				std::uint32_t events{};
				if (fd_status.want_read) events |= EPOLLIN;
				if (fd_status.want_write) events |= EPOLLOUT;
				struct epoll_event event;
				event.events = events;
				event.data.ptr = CAST_TO_VOID_PTR(peer_state);
				int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event);
				epollErrorHandler_(ret, "epoll_ctl");
			} else if (events_[peer_index].events & EPOLLIN) {
				PeerStateHolder *peer_state = 
					CAST_TO_PEERSTATEHOLDER_PTR(events_[peer_index].data.ptr);
				std::shared_ptr<EventLoopBase<PeerState>> ev_loop = this->getSharedPtr();
				FDStatus fd_status = on_read_callback_(peer_state, ev_loop);
				std::uint32_t events{};
				if (fd_status.want_read) events |= EPOLLIN;
				if (fd_status.want_write) events |= EPOLLOUT;
				if (!events) {
					close(peer_state->getFileDescriptor());
					delete CAST_TO_PEERSTATE_PTR(CAST_TO_PEERSTATEHOLDER_PTR(
								events_[peer_index].data.ptr)->getPeerState());
					delete CAST_TO_PEERSTATEHOLDER_PTR(events_[peer_index].data.ptr);
				} else {
					modifyEventForPeer(peer_state->getFileDescriptor(), peer_state, events);
				}
			} else if (events_[peer_index].events & EPOLLOUT) {
				PeerStateHolder *peer_state = 
					CAST_TO_PEERSTATEHOLDER_PTR(events_[peer_index].data.ptr);
				std::shared_ptr<EventLoopBase<PeerState>> ev_loop = this->getSharedPtr();
				FDStatus fd_status = on_write_callback_(peer_state, ev_loop);
				std::uint32_t events{};
				if (fd_status.want_read) events |= EPOLLIN;
				if (fd_status.want_write) events |= EPOLLOUT;
				if (!events) {
					close(peer_state->getFileDescriptor());
					delete CAST_TO_PEERSTATE_PTR(CAST_TO_PEERSTATEHOLDER_PTR(
								events_[peer_index].data.ptr)->getPeerState());
					delete CAST_TO_PEERSTATEHOLDER_PTR(events_[peer_index].data.ptr);
				} else {
					modifyEventForPeer(peer_state->getFileDescriptor(), peer_state, events);
				}
			}
		}
	}
}
// clang-format on

} // namespace blueth::concurrency
