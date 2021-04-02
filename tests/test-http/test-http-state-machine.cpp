#include "HTTPConstants.hpp"
#include <http/HTTPParserStateMachine.hpp>
#include <io/IOBuffer.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace blueth;
TEST(HttpStateMachine, HttpStateMachineGET){
	{ // Valid HTTP GET message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
			http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::ParserState current_state = http::ParserState::RequestLineBegin;
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request =
			http::ParseHTTP1_1RequestMessage(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::ParserState::ParsingDone);
		ASSERT_TRUE(parsed_request.second->constGetHTTPHeaders()->headerCount() == 3);
		ASSERT_TRUE(parsed_request.second->getHTTPVersion() == http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(parsed_request.second->getRequestType() == http::HTTPRequestType::Get);
		ASSERT_TRUE(parsed_request.second->getTargetResource() == "/index.php");
		ASSERT_TRUE(parsed_request.second->getHeaderValue("Accept") == "*/*");
		ASSERT_TRUE(parsed_request.second->getHeaderValue("User-Agent").value() == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request.second->getHeaderValue("Host") == "Proxygen.fb.com");
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
			http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::ParserState current_state = http::ParserState::RequestLineBegin;
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request =
			http::ParseHTTP1_1RequestMessage(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::ParserState::ProtocolError);
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
			http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent : FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::ParserState current_state = http::ParserState::RequestLineBegin;
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request =
			http::ParseHTTP1_1RequestMessage(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::ParserState::ProtocolError);
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
			http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent : FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::ParserState current_state = http::ParserState::RequestLineBegin;
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request =
			http::ParseHTTP1_1RequestMessage(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::ParserState::ProtocolError);
	}
	{ // Valid ASYNC simulation
		std::unique_ptr<http::HTTPRequestMessage> http_message =
			http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n";
		std::string sample_request_remaining = "User-Agent: FB/CXX-Bot/12.32\r\n"
					     	       "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::ParserState current_state = http::ParserState::RequestLineBegin;
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request =
			http::ParseHTTP1_1RequestMessage(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::ParserState::HeaderName);
		io_buffer->appendRawBytes(sample_request_remaining.c_str(), sample_request_remaining.size());
		io_buffer->setStartOffset(sample_request.size());
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request_remaining =
			http::ParseHTTP1_1RequestMessage(io_buffer, parsed_request.first, std::move(parsed_request.second));
		ASSERT_TRUE(parsed_request_remaining.first == http::ParserState::ParsingDone);
		ASSERT_TRUE(parsed_request_remaining.second->constGetHTTPHeaders()->headerCount() == 3);
		ASSERT_TRUE(parsed_request_remaining.second->getHTTPVersion() == http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(parsed_request_remaining.second->getRequestType() == http::HTTPRequestType::Get);
		ASSERT_TRUE(parsed_request_remaining.second->getTargetResource() == "/index.php");
		ASSERT_TRUE(parsed_request_remaining.second->getHeaderValue("Accept") == "*/*");
		ASSERT_TRUE(parsed_request_remaining.second->getHeaderValue("User-Agent") == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request_remaining.second->getHeaderValue("Host") == "Proxygen.fb.com");
	}
	{ // Valid ASYNC simulation
		std::unique_ptr<http::HTTPRequestMessage> http_message =
			http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n";
		std::string sample_request_remaining = "User-Agent: FB/CXX-Bot/12.32\r\n"
						       "Host:";

		std::string sample_request_remaining_two = " Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer = io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(), sample_request.size());
		http::ParserState current_state = http::ParserState::RequestLineBegin;
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request =
			http::ParseHTTP1_1RequestMessage(io_buffer, current_state, std::move(http_message));
		ASSERT_TRUE(parsed_request.first == http::ParserState::HeaderName);
		io_buffer->appendRawBytes(sample_request_remaining.c_str(), sample_request_remaining.size());
		io_buffer->setStartOffset(sample_request.size());
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request_remaining =
			http::ParseHTTP1_1RequestMessage(io_buffer, parsed_request.first, std::move(parsed_request.second));
		io_buffer->appendRawBytes(sample_request_remaining_two.c_str(), sample_request_remaining_two.size());
		io_buffer->modifyStartOffset(sample_request_remaining.size());
		std::pair<http::ParserState, std::unique_ptr<http::HTTPRequestMessage>> parsed_request_remaining_two =
			http::ParseHTTP1_1RequestMessage(io_buffer, parsed_request_remaining.first, std::move(parsed_request_remaining.second));
		ASSERT_TRUE(parsed_request_remaining_two.first == http::ParserState::ParsingDone);
		ASSERT_TRUE(parsed_request_remaining_two.second->constGetHTTPHeaders()->headerCount() == 3);
		ASSERT_TRUE(parsed_request_remaining_two.second->getHTTPVersion() == http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(parsed_request_remaining_two.second->getRequestType() == http::HTTPRequestType::Get);
		ASSERT_TRUE(parsed_request_remaining_two.second->getTargetResource() == "/index.php");
		ASSERT_TRUE(parsed_request_remaining_two.second->getHeaderValue("Accept") == "*/*");
		ASSERT_TRUE(parsed_request_remaining_two.second->getHeaderValue("User-Agent") == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request_remaining_two.second->getHeaderValue("Host") == "Proxygen.fb.com");
	}
}
