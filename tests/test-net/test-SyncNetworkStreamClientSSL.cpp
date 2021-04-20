#include "io/IOBuffer.hpp"
#include "net/NetUtils.hpp"
#include "net/NetworkStream.hpp"
#include "net/Socket.hpp"
#include "net/SyncNetworkStreamClientSSL.hpp"
#include "wolfssl/ssl.h"
#include <cstdint>
#include <cstdio>
#include <gtest/gtest.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

void ssl_server_thread(std::uint16_t, std::string, std::string);

namespace {
static std::string expected = "Hello, from SSL server";
static std::string toWrite = "Hello, from C++ Tester";
} // namespace
TEST(SyncNetworkStreamClientSSL, TestOne) {
	constexpr std::uint16_t server_port = 9876;
	using namespace blueth;
	using ConstIOBuffer = const std::unique_ptr<io::IOBuffer<char>> &;
	std::string cert{"./cert.pem"}, key{"./key.pem"};
	std::thread ssl_serv(ssl_server_thread, server_port, cert, key);
	ssl_serv.detach();
	std::unique_ptr<net::NetworkStream<char>> syncSSLStream =
	    net::SyncNetworkStreamClientSSL::create("localhost", server_port,
						    net::StreamProtocol::TCP);
	EXPECT_TRUE(
	    syncSSLStream->constGetIOBuffer()->getDataSize() == 0 &&
	    syncSSLStream->constGetIOBuffer()->getAvailableSpace() ==
		net::SyncNetworkStreamClientSSL::default_io_buffer_size);
	syncSSLStream->streamWrite(toWrite);
	syncSSLStream->streamRead(1024);
	EXPECT_EQ(syncSSLStream->constGetIOBuffer()->getDataSize(), expected.size());
}

void ssl_server_thread(std::uint16_t port, std::string ssl_cert,
		       std::string ssl_key) {
	WOLFSSL *ssl;
	WOLFSSL_CTX *ctx;
	wolfSSL_Init();
	blueth::net::Socket socket("127.0.0.1", port, 5,
				   blueth::net::Domain::Ipv4,
				   blueth::net::SockType::Stream);
	socket.setSocketOption(blueth::net::SockOptLevel::SocketLevel,
			       blueth::net::SocketOptions::ReuseAddress);
	socket.bindSock();
	if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_server_method())) == nullptr) {
		std::perror("ssl_server_thread wolfSSL_CTX_new()");
		::exit(1);
	}
	if (wolfSSL_CTX_use_certificate_file(ctx, ssl_cert.c_str(),
					     SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		std::perror(
		    "ssl_server_thread wolfSSL_CTX_use_certificate_file");
		::exit(1);
	}
	if (wolfSSL_CTX_use_PrivateKey_file(ctx, ssl_key.c_str(),
					    SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		std::perror(
		    "ssl_server_thread wolfSSL_CTX_use_PrivateKey_file");
		::exit(1);
	}
	sockaddr_in serv_addr;
	sockaddr_in client_addr;
	memset(&serv_addr, 0, sizeof(sockaddr_in));
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	bool shutdown{false};
	while (!shutdown) {
		char buffer[2048] = {0};
		int cli_fd;
		socklen_t size = sizeof(client_addr);
		if ((cli_fd = accept(socket.getFileDescriptor(),
				     (struct sockaddr *)&client_addr, &size)) <
		    0) {
			std::perror("accept()");
			exit(1);
		}
		if((ssl = wolfSSL_new(ctx)) == nullptr){
			fprintf(stderr, "failed to create wolfSSL object\n");
			exit(1);
		}
		wolfSSL_set_fd(ssl, cli_fd);
		int ret = wolfSSL_accept(ssl);
		if (ret != SSL_SUCCESS) {
			fprintf(stderr, "wolfSSL_accept = %d\n", wolfSSL_get_error(ssl, ret));
			exit(1);
		}
		if (wolfSSL_read(ssl, buffer, sizeof(buffer) - 1) < 0) {
			fprintf(stderr, "wolfssl failed to read");
			exit(1);
		}
		if (strncmp(buffer, toWrite.c_str(), toWrite.size()) != 0) {
			std::cout << "test-SyncNetworkStreamClientSSL, client "
				     "sent buffer doesn't match"
				  << std::endl;
		}
		if (wolfSSL_write(ssl, expected.c_str(), expected.size()) !=
		    expected.size()) {
			fprintf(stderr, "failed to write to SSL endpoint");
			exit(1);
		}
		shutdown = true;
	}
	wolfSSL_CTX_free(ctx);
	wolfSSL_Cleanup();
}
