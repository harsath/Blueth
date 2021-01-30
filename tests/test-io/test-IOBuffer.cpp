#include "IOBuffer.hpp"
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

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
	io_buffer->setStartOffset(1);
	ASSERT_EQ(io_buffer->getDataSize(), sample_one_size+sample_two_size-1);
	io_buffer->setEndOffset(-1);
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
	//ASSERT_EQ(io_buffer->getAvailableSpace(), sample_one_size);
}
