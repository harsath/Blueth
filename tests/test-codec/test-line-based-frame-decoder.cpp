#include <codec/LineBasedFrameDecoder.hpp>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

using namespace blueth;
TEST(LineBasedFrameDecoderTest, TestOne) {
	{ // CRLF based line termination
		std::string sample = "first line foo\r\n"
				     "sec line barfasldkfjkl\r\n"
				     "Third\r";
		std::vector<std::string_view> results;
		codec::LineBasedFrameDecoder(
		    (char *)sample.c_str(),
		    (char *)sample.c_str() + sample.size(),
		    codec::TerminatorType::CarriageReturnNewLine, results);
		ASSERT_EQ(results.size(), 2);
		ASSERT_TRUE(std::memcmp(results.at(0).begin(), "first line foo",
					std::strlen("first line foo")) == 0);
		ASSERT_TRUE(
		    std::memcmp(results.at(1).begin(), "sec line barfasldkfjkl",
				std::strlen("sec line barfasldkfjkl")) == 0);
	}
	{ // CRLF based line termination (mixed)
		std::string sample = "first line foo\n\n"
				     "sec line barfasldkfjkl\r\n"
				     "Third\r"
				     "Forth line \r\n";
		std::vector<std::string_view> results;
		codec::LineBasedFrameDecoder(
		    (char *)sample.c_str(),
		    (char *)sample.c_str() + sample.size(),
		    codec::TerminatorType::CarriageReturnNewLine, results);
		const char *first_match =
		    "first line foo\n\nsec line barfasldkfjkl";
		ASSERT_EQ(results.size(), 1);
		ASSERT_TRUE(std::memcmp(results.at(0).begin(), first_match,
					std::strlen(first_match)) == 0);
	}
	{ // NewLine based line terminator
		std::string sample = "first line foo\n\n"
				     "inter line foo\r\r"
				     "some_more lines \n\n"
				     "sec line barfasldkfjkl\r\n"
				     "Third\r";
		std::vector<std::string_view> results;
		codec::LineBasedFrameDecoder(
		    (char *)sample.c_str(),
		    (char *)sample.c_str() + sample.size(),
		    codec::TerminatorType::NewLineNewLine, results);
		ASSERT_EQ(results.size(), 2);
		const char *first_match = "first line foo";
		const char *sec_match = "inter line foo\r\rsome_more lines ";
		ASSERT_TRUE(std::memcmp(results.at(0).begin(), first_match,
					std::strlen(first_match)) == 0);
		ASSERT_TRUE(std::memcmp(results.at(1).begin(), sec_match,
					std::strlen(sec_match)) == 0);
	}
}
