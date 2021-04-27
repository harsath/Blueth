#include "HTTPMessage.hpp"
#include <HTTPConstants.hpp>
#include <gtest/gtest.h>
#include <http/HTTPParserStateMachineResponse.hpp>
#include <io/IOBuffer.hpp>
#include <iostream>

using namespace blueth;
TEST(HttpStateMachineResponse, TestOne) {
	{ // Valid HTTP 301 response message
		std::unique_ptr<http::HTTPResponseMessage> http_message =
		    http::HTTPResponseMessage::create();
		std::string sample_response =
		    "HTTP/1.1 301 Moved Permanently\r\n"
		    "Location: https://www.facebook.com/page.php\r\n"
		    "Content-Type: text/html; charset=UTF-8\r\n"
		    "Date: Sat, 24 Apr 2021 04:00:59 GMT\r\n"
		    "X-Powered-By: Proxygen/FB-CXX\r\n"
		    "Content-Length: 47\r\n\r\n"
		    "<html><h1>Moved somewhere, proxygen</h1></html>";
		std::string msg_body =
		    "<html><h1>Moved somewhere, proxygen</h1></html>";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_response.c_str(),
					  sample_response.size());
		http::ResponseParserState current_state =
		    http::ResponseParserState::ResponseProtocolH;
		std::unique_ptr<http::HTTPResponseMessage> parsed_response =
		    http::ParseHTTP1_1ResponseMessage(io_buffer, current_state,
						      std::move(http_message));
		ASSERT_EQ(
		    parsed_response->getHeaderValue("X-Powered-By").value(),
		    "Proxygen/FB-CXX");
		ASSERT_TRUE(current_state ==
			    http::ResponseParserState::ParsingDone);
		ASSERT_EQ(parsed_response->constGetHTTPHeaders()->headerCount(),
			  5);
		ASSERT_EQ(parsed_response->getHeaderValue("Content-Type"),
			  "text/html; charset=UTF-8");
		ASSERT_TRUE(parsed_response->getResponseCode() ==
			    http::HTTPResponseCodes::MovedPermanently);
		ASSERT_TRUE(parsed_response->getHTTPVersion() ==
			    http::HTTPVersion::HTTP1_1);
		ASSERT_EQ(parsed_response->constGetRawBody()->getDataSize(),
			  msg_body.size());
		ASSERT_TRUE(std::memcmp(parsed_response->constGetRawBody()
					    ->getStartOffsetPointer(),
					msg_body.c_str(),
					msg_body.size()) == 0);
		ASSERT_EQ(
		    std::atoi(parsed_response->getHeaderValue("Content-Length")
				  .value()
				  .c_str()),
		    47);
	}
	{ // Invalid 200 OK message
		std::unique_ptr<http::HTTPResponseMessage> http_message =
		    http::HTTPResponseMessage::create();
		std::string sample_response =
		    "HTTP/1.1 200 OK\r"
		    "Location: www.foobar.com\r\n"
		    "X-Powered-By: Proxygen/FB-CXX\r\n\r\n"
		    "<html><h1>It's a OK response</h1></html>";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_response.c_str(),
					  sample_response.size());
		http::ResponseParserState current_state =
		    http::ResponseParserState::ResponseProtocolH;
		std::unique_ptr<http::HTTPResponseMessage> parsed_response =
		    http::ParseHTTP1_1ResponseMessage(io_buffer, current_state,
						      std::move(http_message));
		ASSERT_TRUE(current_state ==
			    http::ResponseParserState::ProtocolError);
		ASSERT_TRUE(parsed_response->constGetRawBody()->getDataSize() ==
			    0);
	}
	{ // Invalid 200 OK message
		std::unique_ptr<http::HTTPResponseMessage> http_message =
		    http::HTTPResponseMessage::create();
		std::string sample_response =
		    "HTTP/1.1 200 OK\r\n"
		    "Location: www.foobar.com\r\n"
		    "X-Powered-By: Proxygen/FB-CXX\r\r\n"
		    "<html><h1>It's a OK response</h1></html>";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_response.c_str(),
					  sample_response.size());
		http::ResponseParserState current_state =
		    http::ResponseParserState::ResponseProtocolH;
		std::unique_ptr<http::HTTPResponseMessage> parsed_response =
		    http::ParseHTTP1_1ResponseMessage(io_buffer, current_state,
						      std::move(http_message));
		ASSERT_TRUE(current_state ==
			    http::ResponseParserState::ProtocolError);
		ASSERT_TRUE(parsed_response->constGetRawBody()->getDataSize() ==
			    0);
	}
	{ // Valid 200 OK message
		std::unique_ptr<http::HTTPResponseMessage> http_message =
		    http::HTTPResponseMessage::create();
		std::string sample_response =
		    "HTTP/1.1 200 OK\r\n"
		    "Location: www.foobar.com\r\n"
		    "Content-Length: 40\r\n"
		    "X-Powered-By: Proxygen/FB-CXX\r\n\r\n"
		    "<html><h1>It's a OK response</h1></html>";
		std::string msg_body =
		    "<html><h1>It's a OK response</h1></html>";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_response.c_str(),
					  sample_response.size());
		http::ResponseParserState current_state =
		    http::ResponseParserState::ResponseProtocolH;
		std::unique_ptr<http::HTTPResponseMessage> parsed_response =
		    http::ParseHTTP1_1ResponseMessage(io_buffer, current_state,
						      std::move(http_message));
		ASSERT_FALSE(current_state ==
			     http::ResponseParserState::ProtocolError);
		ASSERT_FALSE(
		    parsed_response->constGetRawBody()->getDataSize() == 0);
		ASSERT_TRUE(current_state ==
			    http::ResponseParserState::ParsingDone);
		ASSERT_TRUE(parsed_response->constGetRawBody()->getDataSize() ==
			    msg_body.size());
		ASSERT_TRUE(parsed_response->getResponseCode() ==
			    http::HTTPResponseCodes::Ok);
	}
}
