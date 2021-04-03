#pragma once
#include "internal/EventLoopBase.hpp"
#include "io/IOBuffer.hpp"
#include "net/Socket.hpp"
#include <asm-generic/errno.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <exception>
#include <memory>
#include <new>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

namespace blueth::concurrency {

/// Used with the factory function when creating a new Event-Loop object
enum class EventLoopImpl : std::uint8_t { Epoll };

/**
 * This is the object which will be used by a user when we pass the pointer.
 * When an accept event occured, we pass this object to the registered callbacks
 * and the user must modify the peer_state_ which is type PeerStateHolder to
 * their own custom implementations. We only store the pointer. We free() the
 * pointer when we exit. So the user must also use an object with similar
 * symentics.
 *
 * TODO: What if a user have a complex object which have it's own delete
 * symentics? Can we make them to pass a shared_ptr? Or some other mechanism to
 * allow them to manage their own memory?
 */
template <typename PeerState>
class PeerStateHolder final : public PeerStateHolderBase<PeerState> {
      public:
	PeerStateHolder() {}
	int getFD() override { return fd_; }
	void setFD(int fd) override { fd_ = fd; }
	PeerState *getPeerStatePtr() override { return peer_state_; }
	void setPeerStatePtr(PeerState *ptr) override { peer_state_ = ptr; }

      private:
	int fd_;
	PeerState *peer_state_{nullptr};
};

template <typename PeerState>
class AsyncEpollEventLoop final : public EventLoopBase<PeerState> {
      public:
	AsyncEpollEventLoop(int non_blocking_server_sock,
			    size_t num_event_size) noexcept(false);
	void registerCallbackForEvent(
	    std::function<FDStatus(PeerStateHolderBase<PeerState> *)>,
	    EventType) noexcept(false) override;
	void removePeerFromWatchlist(PeerStateHolderBase<PeerState> *) noexcept(
	    false) override{};
	void startEventloop() noexcept(false) override;
	void modifyEventForPeer(PeerStateHolderBase<PeerState> *) noexcept(
	    false) override {}

      private:
	int server_sock_;
	int epoll_fd_;
	size_t max_events_supported_;
	epoll_event accept_event_;
	epoll_event *events_;
	std::function<FDStatus(PeerStateHolderBase<PeerState> *)>
	    on_accept_callback_;
	std::function<FDStatus(PeerStateHolderBase<PeerState> *)>
	    on_read_callback_;
	std::function<FDStatus(PeerStateHolderBase<PeerState> *)>
	    on_write_callback_;
};

template <typename PeerState>
static std::unique_ptr<EventLoopBase<PeerState>>
EventLoopFactory(EventLoopImpl event_loop, int sock_fd = 0,
		 size_t num_events = 0) {
	if (event_loop == EventLoopImpl::Epoll) {
		return std::make_unique<AsyncEpollEventLoop<PeerState>>(
		    sock_fd, num_events);
	}
	throw new std::invalid_argument(
	    "invalid argument to EventLoopFactory function");
}

// clang-format off
template <typename PeerState>
AsyncEpollEventLoop<PeerState>::AsyncEpollEventLoop(
    int non_blocking_server_sock, size_t max_events_supported) noexcept(false)
    : server_sock_{non_blocking_server_sock}, 
      max_events_supported_{max_events_supported} {
	epoll_fd_ = ::epoll_create1(0);
	if (epoll_fd_ == -1) {
		std::perror("epoll_create1");
		throw new std::runtime_error("epoll_create1");
	}
	accept_event_.events = EPOLLIN;
	accept_event_.data.ptr = new PeerStateHolder<PeerState>();
	((PeerStateHolder<PeerState>*)(accept_event_.data.ptr))->setFD(server_sock_);
	if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_sock_, &accept_event_)) {
		std::perror("epoll_ctl EPOLL_CTL_ADD");
		throw new std::runtime_error("epoll_ctl EPOLL_CTL_ADD");
	}
	events_ =
	    (epoll_event*)calloc(max_events_supported_, sizeof(epoll_event));
	if (events_ == nullptr) {
		std::perror("calloc()");
		throw new std::bad_alloc();
	}
}
// clang-format on

template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::registerCallbackForEvent(
    std::function<FDStatus(PeerStateHolderBase<PeerState> *)> callback,
    EventType event) noexcept(false) {
	if (event == EventType::ReadEvent) {
		on_write_callback_ = std::move(callback);
	} else if (event == EventType::WriteEvent) {
		on_read_callback_ = std::move(callback);
	} else if (event == EventType::AcceptEvent) {
		on_accept_callback_ = std::move(callback);
	} else {
		throw new std::runtime_error(
		    "Invaild argument to callback register function");
	}
}

// clang-format off
template <typename PeerState>
void AsyncEpollEventLoop<PeerState>::startEventloop() noexcept(false) {
	for (;;) {
		// TODO: Make the user specify the timeout value
		int nready =
		    ::epoll_wait(epoll_fd_, events_, max_events_supported_, -1);
		for (size_t i{}; i < nready; ++i) {
			if (events_[i].events & EPOLLIN) {
				if (((PeerStateHolder<PeerState>*)(events_[i].data.ptr))->getFD() == server_sock_) {
					struct sockaddr_in sock_addr;
					socklen_t sockaddr_len = sizeof(struct sockaddr_in);
					int client_sock = accept(server_sock_, (struct sockaddr *)&sock_addr, &sockaddr_len);
					if (client_sock < 0) {
						// Very rare so, let's log it
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							printf("accept returned EAGAIN and EWOULDBLOCK");
							continue;
						}else{
							std::perror("accept");
							throw new std::runtime_error("accept()");
						}
					}
					net::makeSocketNonBlocking(client_sock);
					epoll_event event = {0};
					PeerStateHolder<PeerState> *peer_state = new PeerStateHolder<PeerState>();
					peer_state->setFD(client_sock);
					FDStatus on_accept = this->on_accept_callback_(peer_state);
					event.data.ptr = peer_state;
					if(on_accept.want_read) event.events |= EPOLLIN;
					if(on_accept.want_write) event.events |= EPOLLOUT;
					if(event.events == 0){
						this->removePeerFromWatchlist(peer_state);
						continue;
					}
					if(::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_sock, &event) < 0){
						std::perror("epoll_ctl EPOLL_CTL_ADD");
						throw new std::runtime_error("epoll_ctl");
					}
				}else{
					FDStatus read_return = this->on_read_callback_((PeerStateHolder<PeerState>*)events_[i].data.ptr);
					int peer_fd = ((PeerStateHolder<PeerState>*)events_[i].data.ptr)->getFD();
					epoll_event event = {0};
					event.data.ptr = events_[i].data.ptr;	
					if(read_return.want_read) event.events |= EPOLLIN;
					if(read_return.want_write) event.events |= EPOLLOUT;
					if(event.events == 0){
						this->removePeerFromWatchlist((PeerStateHolder<PeerState>*)events_[i].data.ptr);
						continue;
					}
					if(::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, peer_fd, &event) < 0){
						std::perror("epoll_ctl EPOLL_CTL_MOD");
						throw new std::runtime_error("epoll_ctl EPOLL_CTL_MOD");
					}
				}
			}else if(events_[i].events & EPOLLOUT){
				FDStatus read_return = this->on_write_callback_((PeerStateHolder<PeerState>*)events_[i].data.ptr);
				int peer_fd = ((PeerStateHolder<PeerState>*)events_[i].data.ptr)->getFD();
				epoll_event event = {0};
				event.data.ptr = events_[i].data.ptr;
				if(read_return.want_read) event.events |= EPOLLIN;
				if(read_return.want_write) event.events |= EPOLLOUT;
				if(event.events == 0){
					this->removePeerFromWatchlist((PeerStateHolder<PeerState>*)events_[i].data.ptr);
					continue;
				}
				if(::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, peer_fd, &event)){
					std::perror("epoll_ctl EPOLL_CTL_MOD");
					throw new std::runtime_error("epoll_ctl EPOLL_CTL_ADD");
				}
			}
		}
	}
}
// clang-format on

} // namespace blueth::concurrency
