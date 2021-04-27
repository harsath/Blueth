#include <HTTPConstants.hpp>
#include <gtest/gtest.h>
#include <http/HTTPParserStateMachine.hpp>
#include <io/IOBuffer.hpp>
#include <iostream>

using namespace blueth;
TEST(HttpStateMachine, HttpStateMachineGET) {
	{ // Valid HTTP GET message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
		    http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(),
					  sample_request.size());
		http::ParserState current_state =
		    http::ParserState::RequestLineBegin;
		std::unique_ptr<http::HTTPRequestMessage> parsed_request =
		    http::ParseHTTP1_1RequestMessage(io_buffer, current_state,
						     std::move(http_message));
		ASSERT_TRUE(current_state == http::ParserState::ParsingDone);
		ASSERT_TRUE(
		    parsed_request->constGetHTTPHeaders()->headerCount() == 3);
		ASSERT_TRUE(parsed_request->getHTTPVersion() ==
			    http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(parsed_request->getRequestType() ==
			    http::HTTPRequestType::Get);
		ASSERT_TRUE(parsed_request->getTargetResource() ==
			    "/index.php");
		ASSERT_TRUE(parsed_request->getHeaderValue("Accept") == "*/*");
		ASSERT_TRUE(
		    parsed_request->getHeaderValue("User-Agent").value() ==
		    "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request->getHeaderValue("Host") ==
			    "Proxygen.fb.com");
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
		    http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r"
					     "User-Agent: FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(),
					  sample_request.size());
		http::ParserState current_state =
		    http::ParserState::RequestLineBegin;
		std::unique_ptr<http::HTTPRequestMessage> parsed_request =
		    http::ParseHTTP1_1RequestMessage(io_buffer, current_state,
						     std::move(http_message));
		ASSERT_TRUE(current_state == http::ParserState::ProtocolError);
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
		    http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent : FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(),
					  sample_request.size());
		http::ParserState current_state =
		    http::ParserState::RequestLineBegin;
		std::unique_ptr<http::HTTPRequestMessage> parsed_request =
		    http::ParseHTTP1_1RequestMessage(io_buffer, current_state,
						     std::move(http_message));
		ASSERT_TRUE(current_state == http::ParserState::ProtocolError);
	}
	{ // Invalid HTTP GET Message
		std::unique_ptr<http::HTTPRequestMessage> http_message =
		    http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n"
					     "User-Agent : FB/CXX-Bot/12.32\r\n"
					     "Host: Proxygen.fb.com\r\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(),
					  sample_request.size());
		http::ParserState current_state =
		    http::ParserState::RequestLineBegin;
		std::unique_ptr<http::HTTPRequestMessage> parsed_request =
		    http::ParseHTTP1_1RequestMessage(io_buffer, current_state,
						     std::move(http_message));
		ASSERT_TRUE(current_state == http::ParserState::ProtocolError);
	}
	{ // Valid ASYNC simulation
		std::unique_ptr<http::HTTPRequestMessage> http_message =
		    http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n";
		std::string sample_request_remaining =
		    "User-Agent: FB/CXX-Bot/12.32\r\n"
		    "Host: Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(),
					  sample_request.size());
		http::ParserState current_state =
		    http::ParserState::RequestLineBegin;
		std::unique_ptr<http::HTTPRequestMessage> parsed_request =
		    http::ParseHTTP1_1RequestMessage(io_buffer, current_state,
						     std::move(http_message));
		ASSERT_TRUE(current_state == http::ParserState::HeaderName);
		io_buffer->appendRawBytes(sample_request_remaining.c_str(),
					  sample_request_remaining.size());
		io_buffer->setStartOffset(sample_request.size());
		std::unique_ptr<http::HTTPRequestMessage>
		    parsed_request_remaining = http::ParseHTTP1_1RequestMessage(
			io_buffer, current_state, std::move(parsed_request));
		ASSERT_TRUE(current_state == http::ParserState::ParsingDone);
		ASSERT_TRUE(parsed_request_remaining->constGetHTTPHeaders()
				->headerCount() == 3);
		ASSERT_TRUE(parsed_request_remaining->getHTTPVersion() ==
			    http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(parsed_request_remaining->getRequestType() ==
			    http::HTTPRequestType::Get);
		ASSERT_TRUE(parsed_request_remaining->getTargetResource() ==
			    "/index.php");
		ASSERT_TRUE(parsed_request_remaining->getHeaderValue(
				"Accept") == "*/*");
		ASSERT_TRUE(parsed_request_remaining->getHeaderValue(
				"User-Agent") == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request_remaining->getHeaderValue("Host") ==
			    "Proxygen.fb.com");
	}
	{ // Valid ASYNC simulation
		std::unique_ptr<http::HTTPRequestMessage> http_message =
		    http::HTTPRequestMessage::create();
		std::string sample_request = "GET /index.php HTTP/1.1\r\n"
					     "Accept: */*\r\n";
		std::string sample_request_remaining =
		    "User-Agent: FB/CXX-Bot/12.32\r\n"
		    "Host:";

		std::string sample_request_remaining_two =
		    " Proxygen.fb.com\r\n\r\n";
		std::unique_ptr<io::IOBuffer<char>> io_buffer =
		    io::IOBuffer<char>::create(2048);
		io_buffer->appendRawBytes(sample_request.c_str(),
					  sample_request.size());
		http::ParserState current_state =
		    http::ParserState::RequestLineBegin;
		std::unique_ptr<http::HTTPRequestMessage> parsed_request =
		    http::ParseHTTP1_1RequestMessage(io_buffer, current_state,
						     std::move(http_message));
		ASSERT_TRUE(current_state == http::ParserState::HeaderName);
		io_buffer->appendRawBytes(sample_request_remaining.c_str(),
					  sample_request_remaining.size());
		io_buffer->setStartOffset(sample_request.size());
		std::unique_ptr<http::HTTPRequestMessage>
		    parsed_request_remaining = http::ParseHTTP1_1RequestMessage(
			io_buffer, current_state, std::move(parsed_request));
		io_buffer->appendRawBytes(sample_request_remaining_two.c_str(),
					  sample_request_remaining_two.size());
		io_buffer->modifyStartOffset(sample_request_remaining.size());
		std::unique_ptr<http::HTTPRequestMessage>
		    parsed_request_remaining_two =
			http::ParseHTTP1_1RequestMessage(
			    io_buffer, current_state,
			    std::move(parsed_request_remaining));
		ASSERT_TRUE(current_state == http::ParserState::ParsingDone);
		ASSERT_TRUE(parsed_request_remaining_two->constGetHTTPHeaders()
				->headerCount() == 3);
		ASSERT_TRUE(parsed_request_remaining_two->getHTTPVersion() ==
			    http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(parsed_request_remaining_two->getRequestType() ==
			    http::HTTPRequestType::Get);
		ASSERT_TRUE(parsed_request_remaining_two->getTargetResource() ==
			    "/index.php");
		ASSERT_TRUE(parsed_request_remaining_two->getHeaderValue(
				"Accept") == "*/*");
		ASSERT_TRUE(parsed_request_remaining_two->getHeaderValue(
				"User-Agent") == "FB/CXX-Bot/12.32");
		ASSERT_TRUE(parsed_request_remaining_two->getHeaderValue(
				"Host") == "Proxygen.fb.com");
	}
}
