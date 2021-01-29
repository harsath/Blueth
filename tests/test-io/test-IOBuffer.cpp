#include "IOBuffer.hpp"
#include <cstring>
#include <gtest/gtest.h>

TEST(IOBufferTest, IOBuffer){
	std::unique_ptr<io::IOBuffer<char>> io_buffer = 
		io::IOBuffer<char>::create(100);
	ASSERT_EQ(100, io_buffer->getCapacity());
	ASSERT_TRUE((io_buffer->getStartOffset() == 0 && io_buffer->getEndOffset() == 0));
	ASSERT_EQ(io_buffer->getStartOffsetPointer(), io_buffer->getBuffer());
	const char* sample_one = "Hello World";
	std::size_t sample_one_size = std::strlen(sample_one);
	io_buffer->appendRawBytes(sample_one, sample_one_size);
	ASSERT_EQ(io_buffer->getDataSize(), sample_one_size);
	ASSERT_EQ(*(io_buffer->getEndOffsetPointer()-1), 'd');
	ASSERT_EQ(*io_buffer->getStartOffsetPointer(), 'H');
	ASSERT_EQ(std::memcmp(sample_one, io_buffer->getStartOffsetPointer(), sample_one_size), 0);	
	ASSERT_EQ(std::memcmp("ello World", io_buffer->getOffset(1, sample_one_size-1).first, sample_one_size-1), 0);	
}
