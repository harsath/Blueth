#include "io/IOBuffer.hpp"
#include "net/NetworkStream.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <net/SyncNetworkStreamClient.hpp>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

TEST(SyncNetworkStreamClientTest, TestOne) {
	constexpr std::uint16_t socket_port = 1234;

	std::thread server_thread([socket_port]() {
		std::string send_message = "Hello, from server thread";
		std::string expected_message = "Hello, from cxx thread";
		int socket_fd, client_fd;
		sockaddr_in server, client;
		char client_message[2048];
		bzero(client_message, sizeof(client_message));
		socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
		if (socket_fd == -1) {
			std::perror("socket");
			::exit(EXIT_FAILURE);
		}
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = htonl(INADDR_ANY);
		server.sin_port = htons(socket_port);
		if (::bind(socket_fd, (sockaddr *)&server, sizeof(server)) <
		    0) {
			std::perror("bind");
			::exit(EXIT_FAILURE);
		}
		int ret = ::listen(socket_fd, 3);
		if (ret != 0) {
			std::perror("listen");
			::exit(EXIT_FAILURE);
		}
		socklen_t len = sizeof(sockaddr_in);
		client_fd =
		    ::accept(socket_fd, (sockaddr *)&client, (socklen_t *)&len);
		if (client_fd < 0) {
			std::perror("accept");
			::exit(EXIT_FAILURE);
		}
		size_t read_size = ::recv(client_fd, client_message,
					  sizeof(client_message), 0);
		if (read_size < 0) {
			std::perror("recv");
			::exit(EXIT_FAILURE);
		}
		ASSERT_TRUE(std::memcmp(client_message,
					expected_message.c_str(),
					expected_message.size()) == 0);
		int send_size = ::send(client_fd, send_message.c_str(),
				       send_message.size(), 0);
		ASSERT_EQ(send_size, send_message.size());
		::close(client_fd);
		::close(socket_fd);
	});
	using namespace blueth;
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2000ms);

	std::string expected_message = "Hello, from server thread";
	std::string send_message = "Hello, from cxx thread";
	using ConstIOBuffer = const std::unique_ptr<io::IOBuffer<char>> &;
	std::unique_ptr<net::NetworkStream<char>> syncNetStream =
	    net::SyncNetworkStreamClient::create("127.0.0.1", socket_port,
						 net::StreamProtocol::TCP);
	EXPECT_TRUE(syncNetStream->constGetIOBuffer()->getDataSize() == 0 &&
		    syncNetStream->constGetIOBuffer()->getAvailableSpace() ==
			net::SyncNetworkStreamClient::default_io_buffer_size);
	syncNetStream->streamWrite(send_message);
	syncNetStream->streamRead(50);
	ConstIOBuffer io_buffer = syncNetStream->constGetIOBuffer();
	EXPECT_TRUE(io_buffer->getDataSize() == expected_message.size());
	char read_buffer[io_buffer->getDataSize()];
	memcpy(read_buffer, io_buffer->getStartOffsetPointer(),
	       io_buffer->getDataSize());
	EXPECT_TRUE(std::memcmp(read_buffer, expected_message.c_str(),
				io_buffer->getDataSize()) == 0);
	server_thread.join();
}
