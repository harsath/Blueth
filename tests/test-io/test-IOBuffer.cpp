#include "IOBuffer.hpp"
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <utility>

using namespace blueth;
TEST(IOBufferTestOne, IOBuffer){
	std::unique_ptr<io::IOBuffer<char>> io_buffer = 
		io::IOBuffer<char>::create(100);
	ASSERT_EQ(100, io_buffer->getCapacity());
	ASSERT_TRUE((io_buffer->getStartOffset() == 0 && io_buffer->getEndOffset() == 0));
	ASSERT_EQ(io_buffer->getStartOffsetPointer(), io_buffer->getBuffer());
	const char* sample_one = "Hello World";
	std::size_t sample_one_size = std::strlen(sample_one);
	io_buffer->appendRawBytes(sample_one, sample_one_size);
	ASSERT_EQ(io_buffer->getDataSize(), sample_one_size);
	ASSERT_TRUE(io_buffer->getStartOffset() == 0 && io_buffer->getEndOffset() == (sample_one_size));
	ASSERT_EQ(*(io_buffer->getEndOffsetPointer()-1), 'd');
	ASSERT_EQ(*io_buffer->getStartOffsetPointer(), 'H');
	ASSERT_EQ(std::memcmp(sample_one, io_buffer->getStartOffsetPointer(), sample_one_size), 0);	
	ASSERT_EQ(std::memcmp("ello World", io_buffer->getOffset(1, sample_one_size-1).first, sample_one_size-1), 0);	

	const char* sample_two = "Foo bar";
	std::size_t sample_two_size = std::strlen(sample_two);
	io_buffer->appendRawBytes(sample_two, sample_two_size);
	ASSERT_EQ(io_buffer->getDataSize(), sample_one_size+sample_two_size);
	ASSERT_EQ(*(io_buffer->getEndOffsetPointer()-1), 'r');
	io_buffer->modifyStartOffset(1);
	ASSERT_EQ(io_buffer->getDataSize(), sample_one_size+sample_two_size-1);
	io_buffer->modifyEndOffset(-1);
	ASSERT_EQ(io_buffer->getDataSize(), sample_one_size+sample_two_size-2);
	ASSERT_EQ(*io_buffer->getStartOffsetPointer(), 'e');
	ASSERT_EQ(*(io_buffer->getEndOffsetPointer()-1), 'a');
	ASSERT_EQ(*io_buffer->getBuffer(), 'H');
	ASSERT_EQ(*io_buffer->getBufferEnd(), '\0'); // Uninitilized data
}

TEST(IOBufferTestTwo, IOBuffer){
	std::unique_ptr<io::IOBuffer<std::uint8_t>> io_buffer = 
		io::IOBuffer<std::uint8_t>::create(10);
	const std::uint8_t* sample_one = (std::uint8_t*)"Hello";
	std::size_t sample_one_size = std::strlen((const char*)sample_one);
	io_buffer->appendRawBytes(sample_one, sample_one_size);
	ASSERT_EQ(*((io_buffer->getEndOffsetPointer())-1), 'o');
	ASSERT_EQ(io_buffer->getEndOffset(), 5);
	ASSERT_EQ(io_buffer->getAvailableSpace(), 5);
	std::unique_ptr<io::IOBuffer<std::uint8_t>> io_buffer_two =
		io::IOBuffer<std::uint8_t>::create(20);
	io_buffer_two->appendRawBytes(*io_buffer);
	const std::uint8_t* sample_two = (std::uint8_t*)" World";
	std::size_t sample_two_size = std::strlen((const char*)sample_two);
	io_buffer_two->appendRawBytes(sample_two, sample_two_size);

	ASSERT_EQ(io_buffer_two->getDataSize(), sample_one_size + sample_two_size);
	ASSERT_EQ(std::memcmp("Hello World", io_buffer_two->getStartOffsetPointer(), io_buffer_two->getDataSize()), 0);
	ASSERT_EQ(*(io_buffer_two->getEndOffsetPointer()-1), 'd');
}

TEST(IOBufferTestHTTPMessage, IOBuffer){
	std::unique_ptr<io::IOBuffer<char>> io_buffer = 
		io::IOBuffer<char>::create(20);
	std::string http_sample = 
		"GET / HTTP/1.1\n\r"
		"Host: www.example.com\n\r"
		"User-Agent: FireFox(Mobile client)\n\r"
		"Accept: */*\n\r\n\r";
	io_buffer->appendRawBytes(http_sample.c_str(), http_sample.size());
	ASSERT_EQ(*((io_buffer->getEndOffsetPointer())-1), '\r');
	ASSERT_EQ(io_buffer->getDataSize(), http_sample.size());
	ASSERT_EQ(*io_buffer->getStartOffsetPointer(), 'G');
	io_buffer->modifyStartOffset(1);  // We sent 1 byte through non-blocking net socket
	ASSERT_EQ(io_buffer->getDataSize(), http_sample.size()-1);
	ASSERT_EQ(*io_buffer->getStartOffsetPointer(), 'E');
	std::pair<const char*, const char*> offset_pair = io_buffer->getOffset(10);
	const char* offset_pair_eq = "ET / HTTP/";
	ASSERT_EQ(std::memcmp(offset_pair_eq, offset_pair.first, 10), 0);

	io_buffer->modifyStartOffset(std::strlen(offset_pair_eq));  // Let's pretend, we sent these 10 bytes through net socket
	std::pair<const char*, const char*> offset_pair_1 = io_buffer->getOffset(10);
	const char* offset_pair_eq_1 = "1.1\n\rHost:";
	ASSERT_EQ(std::memcmp(offset_pair_eq_1, offset_pair_1.first, 10), 0);

	io_buffer->modifyStartOffset(std::strlen(offset_pair_eq_1));
	ASSERT_EQ(*io_buffer->begin(), ' ');
	io_buffer->modifyStartOffset(1);
	ASSERT_EQ(*io_buffer->begin(), 'w');
	ASSERT_TRUE((*(io_buffer->end()-1) == '\r' && *(io_buffer->cend()-1) == '\r' && *io_buffer->cbegin() == 'w'));
}
