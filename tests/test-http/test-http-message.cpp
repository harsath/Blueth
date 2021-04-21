#include "io/IOBuffer.hpp"
#include <gtest/gtest.h>
#include <http/HTTPConstants.hpp>
#include <http/HTTPMessage.hpp>
#include <memory>
#include <string>

using namespace blueth;
TEST(HttpTestOne, HttpGet) {
	{
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
		http_message->setRequestType(http::HTTPRequestType::Get);
		http_message->pushBackTargetResource('/');
		http_message->pushBackTargetResource('i');
		http_message->pushBackTargetResource('n');
		http_message->setHTTPTargetResource("dex.p");
		http_message->pushBackTargetResource('h');
		http_message->pushBackTargetResource('p');
		http_message->setHTTPVersion(http::HTTPVersion::HTTP1_1);

		http_message->addHeader("Host", "Proxygen.fb.com");
		http_message->addHeader("User-Agent", "FB/CXX-Bot/12.32");
		http_message->addHeader("Accept", "*/*");

		ASSERT_TRUE(sample_request == http_message->buildRawMessage());
		ASSERT_TRUE(http_message->getHTTPVersion() ==
			    http::HTTPVersion::HTTP1_1);
		ASSERT_TRUE(http_message->getRequestType() ==
			    http::HTTPRequestType::Get);
		ASSERT_TRUE(http_message->getTargetResource() == "/index.php");
		ASSERT_TRUE(
		    http_message->constGetHTTPHeaders()->headerCount() == 3);
		ASSERT_TRUE(http_message->getHeaderValue("Host") ==
			    "Proxygen.fb.com");
	}
	{ // valid CONNECT message
		http::HTTPRequestMessage connect_request_message;
		connect_request_message.setRequestType(
		    http::HTTPRequestType::Connect);
		connect_request_message.setHTTPTargetResource(
		    "www.foo.com:443");
		connect_request_message.setHTTPVersion(
		    http::HTTPVersion::HTTP1_1);
		connect_request_message.addHeader("Host", "www.foo.com:443");
		connect_request_message.addHeader("User-Agent",
						  "blueth/http-client");
		connect_request_message.addHeader("Proxy-Connection",
						  "Keep-Alive");
		std::string expected = "CONNECT www.foo.com:443 HTTP/1.1\r\n"
				       "User-Agent: blueth/http-client\r\n"
				       "Proxy-Connection: Keep-Alive\r\n"
				       "Host: www.foo.com:443\r\n\r\n";
		ASSERT_EQ(connect_request_message.buildRawMessage(), expected);
	}
}
