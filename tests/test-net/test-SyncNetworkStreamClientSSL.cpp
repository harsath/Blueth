#include "io/IOBuffer.hpp"
#include "net/NetUtils.hpp"
#include "net/NetworkStream.hpp"
#include "net/Socket.hpp"
#include "net/SyncNetworkStreamClientSSL.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

void ssl_server_thread();

namespace {
std::string server_message = "Hello, from SSL server";
std::string client_message = "Hello, from C++ Tester";
std::string server_cert = "./localtester.net.crt";
std::string server_key = "./localtester.net.key";
std::string ca_cert = "ca.crt";
constexpr std::uint16_t server_port = 9876;
const char *server_address = "127.0.0.1";
#define PERROR_EXIT(message)                                                   \
	std::perror(message);                                                  \
	::exit(EXIT_FAILURE);
} // namespace

TEST(SyncNetworkStreamClientSSL, TestOne) {
	using namespace blueth;
	using ConstIOBuffer = const std::unique_ptr<io::IOBuffer<char>> &;
	std::thread ssl_serv(ssl_server_thread);
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2000ms);
	std::unique_ptr<net::NetworkStream<char>> syncSSLStream =
	    net::SyncNetworkStreamClientSSL::create(
		server_address, server_port, net::StreamProtocol::TCP, ca_cert);
	syncSSLStream->streamWrite(client_message);
	syncSSLStream->streamRead(50);
	const std::unique_ptr<io::IOBuffer<char>> &io_buffer =
	    syncSSLStream->constGetIOBuffer();
	EXPECT_EQ(io_buffer->getDataSize(), server_message.size());
	EXPECT_EQ(std::strncmp(io_buffer->getStartOffsetPointer(),
			       server_message.c_str(), server_message.size()),
		  0);
	ssl_serv.join();
}

void ssl_server_thread() {
	int socket_fd;
	sockaddr_in server_addr;
	sockaddr_in client_addr;
	socklen_t size = sizeof(client_addr);
	char buffer[1024] = {0};
	size_t len;
	bool shutdown = false;
	int ret;

	WOLFSSL_CTX *ctx;
	WOLFSSL *ssl;
	wolfSSL_Init();

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		PERROR_EXIT("socket");
	}

	if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_server_method())) == nullptr) {
		PERROR_EXIT("wolfSSL_CTX_new");
	}

	if (wolfSSL_CTX_use_certificate_file(ctx, server_cert.c_str(),
					     SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		PERROR_EXIT("wolfSSL_CTX_use_certificate_file");
	}

	if (wolfSSL_CTX_use_PrivateKey_file(ctx, server_key.c_str(),
					    SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		PERROR_EXIT("wolfSSL_CTX_use_PrivateKey_file");
	}

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) == -1) {
		PERROR_EXIT("inet_pton");
	}

	if (bind(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr)) ==
	    -1) {
		PERROR_EXIT("bind");
	}

	if (listen(socket_fd, 5) == -1) { PERROR_EXIT("listen"); }

	while (!shutdown) {
		int client_fd;
		if ((client_fd = accept(socket_fd, (sockaddr *)&client_addr,
					&size)) == -1) {
			PERROR_EXIT("accept");
		}

		if ((ssl = wolfSSL_new(ctx)) == nullptr) {
			PERROR_EXIT("wolfSSL_new");
		}

		wolfSSL_set_fd(ssl, client_fd);
		int ret = wolfSSL_accept(ssl);
		if (ret != SSL_SUCCESS) { PERROR_EXIT("wolfSSL_accept"); }

		if (wolfSSL_read(ssl, buffer, sizeof(buffer) - 1) == -1) {
			PERROR_EXIT("wolfSSL_read");
		}

		if (wolfSSL_write(ssl, server_message.c_str(),
				  server_message.size()) !=
		    server_message.size()) {
			PERROR_EXIT("wolfSSL_write");
		}

		EXPECT_EQ(std::strncmp(client_message.c_str(), buffer,
				       client_message.size()),
			  0);
		shutdown = true;
	}
}
