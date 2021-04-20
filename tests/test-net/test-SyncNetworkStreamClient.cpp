#include "io/IOBuffer.hpp"
#include "net/NetworkStream.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <net/SyncNetworkStreamClient.hpp>

TEST(SyncNetworkStreamClientTest, TestOne) {
	constexpr std::uint16_t perl_port = 1234;
	std::system("perl ../tests/scripts/sync_client_netstream_test.pl &");
	using namespace blueth;
	std::string perlSciptExpected = "Hello, it's from Perl";
	using ConstIOBuffer = const std::unique_ptr<io::IOBuffer<char>> &;
	std::unique_ptr<net::NetworkStream<char>> syncNetStream =
	    net::SyncNetworkStreamClient::create("localhost", perl_port,
						 net::StreamProtocol::TCP);
	std::string writeData = "Hey, this is client";
	EXPECT_TRUE(syncNetStream->constGetIOBuffer()->getDataSize() == 0 &&
		    syncNetStream->constGetIOBuffer()->getAvailableSpace() ==
			net::SyncNetworkStreamClient::default_io_buffer_size);
	syncNetStream->streamWrite(writeData);
	syncNetStream->streamRead(50);
	ConstIOBuffer io_buffer = syncNetStream->constGetIOBuffer();
	EXPECT_TRUE(io_buffer->getDataSize() == perlSciptExpected.size());
	char read_buffer[io_buffer->getDataSize()];
	memcpy(read_buffer, io_buffer->getStartOffsetPointer(),
	       io_buffer->getDataSize());
	EXPECT_TRUE(std::memcmp(read_buffer, perlSciptExpected.c_str(),
				io_buffer->getDataSize()) == 0);
}
