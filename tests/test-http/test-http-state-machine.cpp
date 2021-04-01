#include "http_constants.hpp"
#include <http/http_parser_state_machine.hpp>
#include <io/IOBuffer.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace blueth;
TEST(HttpStateMachine, HttpStateMachineGET){
	{ // Valid HTTP GET message
		std::unique_ptr<http::http_request_message> http_message =
			http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request =
			http::parse_http_1_1_request_message(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::parser_state::parsing_done);
		ASSERT_TRUE(parsed_request.second->const_get_http_headers()->header_count() == 3);
		ASSERT_TRUE(parsed_request.second->get_http_version() == http::http_version::http_1_1);
		ASSERT_TRUE(parsed_request.second->get_request_type() == http::http_request_type::get);
		ASSERT_TRUE(parsed_request.second->get_target_resource() == "/index.php");
		ASSERT_TRUE(parsed_request.second->get_header_value("Accept") == "*/*");
		ASSERT_TRUE(parsed_request.second->get_header_value("User-Agent").value() == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request.second->get_header_value("Host") == "Proxygen.fb.com");
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::http_request_message> http_message =
			http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request =
			http::parse_http_1_1_request_message(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::parser_state::protocol_error);
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::http_request_message> http_message =
			http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent : FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request =
			http::parse_http_1_1_request_message(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::parser_state::protocol_error);
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::http_request_message> http_message =
			http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent : FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request =
			http::parse_http_1_1_request_message(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::parser_state::protocol_error);
	}
	{ // Valid ASYNC simulation
		std::unique_ptr<http::http_request_message> http_message =
			http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n";
		std::string sample_request_remaining = "User-Agent: FB/CXX-Bot/12.32\r\n"
					     	       "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request =
			http::parse_http_1_1_request_message(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::parser_state::header_name);
		io_buffer->appendRawBytes(sample_request_remaining.c_str(), sample_request_remaining.size());
		io_buffer->setStartOffset(sample_request.size());
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request_remaining =
			http::parse_http_1_1_request_message(io_buffer, parsed_request.first, std::move(parsed_request.second));
		ASSERT_TRUE(parsed_request_remaining.first == http::parser_state::parsing_done);
		ASSERT_TRUE(parsed_request_remaining.second->const_get_http_headers()->header_count() == 3);
		ASSERT_TRUE(parsed_request_remaining.second->get_http_version() == http::http_version::http_1_1);
		ASSERT_TRUE(parsed_request_remaining.second->get_request_type() == http::http_request_type::get);
		ASSERT_TRUE(parsed_request_remaining.second->get_target_resource() == "/index.php");
		ASSERT_TRUE(parsed_request_remaining.second->get_header_value("Accept") == "*/*");
		ASSERT_TRUE(parsed_request_remaining.second->get_header_value("User-Agent") == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request_remaining.second->get_header_value("Host") == "Proxygen.fb.com");
	}
	{ // Valid ASYNC simulation
		std::unique_ptr<http::http_request_message> http_message =
			http::http_request_message::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n";
		std::string sample_request_remaining = "User-Agent: FB/CXX-Bot/12.32\r\n"
						       "Host:";

		std::string sample_request_remaining_two = " Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::parser_state current_state = http::parser_state::request_line_begin;
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request =
			http::parse_http_1_1_request_message(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::parser_state::header_name);
		io_buffer->appendRawBytes(sample_request_remaining.c_str(), sample_request_remaining.size());
		io_buffer->setStartOffset(sample_request.size());
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request_remaining =
			http::parse_http_1_1_request_message(io_buffer, parsed_request.first, std::move(parsed_request.second));
		io_buffer->appendRawBytes(sample_request_remaining_two.c_str(), sample_request_remaining_two.size());
		io_buffer->modifyStartOffset(sample_request_remaining.size());
		std::pair<http::parser_state, std::unique_ptr<http::http_request_message>> parsed_request_remaining_two =
			http::parse_http_1_1_request_message(io_buffer, parsed_request_remaining.first, std::move(parsed_request_remaining.second));
		ASSERT_TRUE(parsed_request_remaining_two.first == http::parser_state::parsing_done);
		ASSERT_TRUE(parsed_request_remaining_two.second->const_get_http_headers()->header_count() == 3);
		ASSERT_TRUE(parsed_request_remaining_two.second->get_http_version() == http::http_version::http_1_1);
		ASSERT_TRUE(parsed_request_remaining_two.second->get_request_type() == http::http_request_type::get);
		ASSERT_TRUE(parsed_request_remaining_two.second->get_target_resource() == "/index.php");
		ASSERT_TRUE(parsed_request_remaining_two.second->get_header_value("Accept") == "*/*");
		ASSERT_TRUE(parsed_request_remaining_two.second->get_header_value("User-Agent") == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request_remaining_two.second->get_header_value("Host") == "Proxygen.fb.com");
	}
}
