#include "io/IOBuffer.hpp"
#include <gtest/gtest.h>
#include <http/http_message.hpp>
#include <http/http_constants.hpp>
#include <memory>
#include <string>

using namespace blueth;
TEST(HttpTestOne, HttpGet) {
	{
		std::unique_ptr<http::http_request_message> http_message =
		    http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;	
		http_message->set_request_type(http::http_request_type::get);		
		http_message->push_back_target_resource('/');
		http_message->push_back_target_resource('i');
		http_message->push_back_target_resource('n');
		http_message->set_http_target_resource("dex.p");
		http_message->push_back_target_resource('h');
		http_message->push_back_target_resource('p');
		http_message->set_http_version(http::http_version::http_1_1);

		http_message->add_header("Host", "Proxygen.fb.com");
		http_message->add_header("User-Agent", "FB/CXX-Bot/12.32");
		http_message->add_header("Accept", "*/*");

		ASSERT_TRUE(sample_request == http_message->build_raw_message());
		ASSERT_TRUE(http_message->get_http_version() == http::http_version::http_1_1);
		ASSERT_TRUE(http_message->get_request_type() == http::http_request_type::get);
		ASSERT_TRUE(http_message->get_target_resource() == "/index.php");
		ASSERT_TRUE(http_message->const_get_http_headers()->header_count() == 3);
		ASSERT_TRUE(http_message->get_header_value("Host") == "Proxygen.fb.com");
	}
}
