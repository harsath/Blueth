#include "HTTPConstants.hpp"
#include "http/HTTPMessage.hpp"
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>

TEST(HTTPResponseMessageTest, TestOne) {
	using namespace blueth;
	{
		std::unique_ptr<http::HTTPResponseMessage> message =
		    http::HTTPResponseMessage::create();
		std::string expected =
		    "HTTP/1.1 301 Moved Permanently\r\n"
		    "Cache-Control: public, max-age=2592000\r\n"
		    "Content-Type: text/html; charset=UTF-8\r\n"
		    "Powered-By: Proxygen/FB-CXX\r\n"
		    "Location: https://fb.com/endpoint.php\r\n\r\n"
		    "<html>it's a 301 response</html>";
		message->setHTTPVersion(http::HTTPVersion::HTTP1_1);
		message->pushBackResponseCode('3');
		message->pushBackResponseCode('0');
		message->pushBackResponseCode('1');
		ASSERT_TRUE(std::string{"301"} ==
			    message->getTempStatusCode());
		message->setResponseCode(static_cast<http::HTTPResponseCodes>(
		    std::atoi(message->getTempStatusCode())));
		message->addHeader("Location", "https://fb.com/endpoint.php");
		message->addHeader("Content-Type", "text/html; charset=UTF-8");
		message->addHeader("Cache-Control", "public, max-age=2592000");
		message->pushBackHeaderName('P');
		message->pushBackHeaderName('o');
		message->pushBackHeaderName('w');
		message->pushBackHeaderName('e');
		message->pushBackHeaderName('r');
		message->pushBackHeaderName('e');
		message->pushBackHeaderName('d');
		message->pushBackHeaderName('-');
		message->pushBackHeaderName('B');
		message->pushBackHeaderName('y');
		message->pushBackHeaderValue('P');
		message->pushBackHeaderValue('r');
		message->pushBackHeaderValue('o');
		message->pushBackHeaderValue('x');
		message->pushBackHeaderValue('y');
		message->pushBackHeaderValue('g');
		message->pushBackHeaderValue('e');
		message->pushBackHeaderValue('n');
		message->pushBackHeaderValue('/');
		message->pushBackHeaderValue('F');
		message->pushBackHeaderValue('B');
		message->pushBackHeaderValue('-');
		message->pushBackHeaderValue('C');
		message->pushBackHeaderValue('X');
		message->pushBackHeaderValue('X');
		message->addTempHeadersHolderToMessage();
		message->pushBackRawBody("<html>it's a 301 response</html>");
		ASSERT_EQ(message->buildRawMessage(), expected);
	}
}
