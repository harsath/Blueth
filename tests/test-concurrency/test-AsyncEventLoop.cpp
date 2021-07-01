#include "concurrency/AsyncEventLoop.hpp"
#include "concurrency/internal/EventLoopBase.hpp"
#include "io/IOBuffer.hpp"
#include "net/NetworkStream.hpp"
#include "net/Socket.hpp"
#include "net/SyncNetworkStreamClient.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

using namespace blueth;

const char *server_address = "127.0.0.1";
const std::uint16_t server_port = 9090;
const size_t epoll_size = 50;
const int server_timeout = 3000;
const int server_backlog = 50;
const std::string client_reply = "Hello, from client";
const std::string server_reply = "Hello, from server";

enum class PeerConnectionState { NotConnected, Connected, Handshake, Closed };

class PeerState {
      public:
	PeerState() : peer_state{PeerConnectionState::NotConnected} {
		io_buffer = std::make_shared<io::IOBuffer<char>>(1048);
	}
	int peer_fingerprint;
	PeerConnectionState peer_state;
	std::shared_ptr<io::IOBuffer<char>> io_buffer;
};

concurrency::FDStatus
on_write(concurrency::PeerStateHolder *peer_state_holder,
	 std::shared_ptr<concurrency::EventLoopBase<PeerState>> io_context) {
	PeerState *peer_state =
	    CAST_TO_PEERSTATE_PTR(peer_state_holder->getPeerState());
	peer_state->io_buffer->appendRawBytes(server_reply.c_str(),
					      server_reply.size());
	int written_bytes =
	    io_context->writeToPeer(peer_state_holder, peer_state->io_buffer);
	EXPECT_EQ(written_bytes, server_reply.size());
	std::cout << "written: " << written_bytes << std::endl;
	return concurrency::WantWrite;
}

concurrency::FDStatus
on_read(concurrency::PeerStateHolder *peer_state_holder,
	std::shared_ptr<concurrency::EventLoopBase<PeerState>> io_context) {
	PeerState *peer_state =
	    CAST_TO_PEERSTATE_PTR(peer_state_holder->getPeerState());
	io_context->readFromPeer(peer_state_holder, peer_state->io_buffer);
	std::string read_data{peer_state->io_buffer->getStartOffsetPointer(),
			      peer_state->io_buffer->getEndOffsetPointer()};
	std::cout << "read: " << read_data << std::endl;
	EXPECT_EQ(read_data, client_reply);
	return concurrency::WantNoReadWrite;
}

concurrency::FDStatus
on_accept(concurrency::PeerStateHolder *peer_state_holder,
	  std::shared_ptr<concurrency::EventLoopBase<PeerState>> io_context) {
	return concurrency::WantRead;
}

TEST(AsyncEventLoopTest, EpollTest) {
	std::shared_ptr<concurrency::EventLoopBase<PeerState>> event_loop =
	    concurrency::AsyncEpollEventLoop<PeerState>::create(
		server_address, server_port, epoll_size, server_backlog,
		server_timeout);
	event_loop->registerCallbackForEvent(
	    on_write, concurrency::EventType::WriteEvent);
	event_loop->registerCallbackForEvent(on_read,
					     concurrency::EventType::ReadEvent);
	event_loop->registerCallbackForEvent(
	    on_accept, concurrency::EventType::AcceptEvent);
	std::thread client_thread([=]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1000ms);
		std::unique_ptr<net::NetworkStream<char>> client =
		    net::SyncNetworkStreamClient::create(
			server_address, server_port, net::StreamProtocol::TCP);
		client->streamWrite(client_reply);
		client->streamRead(50);
		std::string read_data{
		    client->constGetIOBuffer()->getStartOffsetPointer(),
		    client->constGetIOBuffer()->getEndOffsetPointer()};
		EXPECT_EQ(read_data, server_reply);
	});
	event_loop->startEventloop();
	client_thread.join();
}
