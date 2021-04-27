#pragma once
#include "HTTPMessage.hpp"
#include "HTTPParserCommon.hpp"
#include "common.hpp"
#include "http/HTTPConstants.hpp"
#include "io/IOBuffer.hpp"
#include <cctype>
#include <memory>
#include <utility>

namespace blueth::http {

// Parser for HTTP Requet Message
inline std::unique_ptr<HTTPRequestMessage>
ParseHTTP1_1RequestMessage(
    const std::unique_ptr<io::IOBuffer<char>> &request_message,
    ParserState &current_state,
    std::unique_ptr<HTTPRequestMessage> http_message) {
	const char *start_buffer = request_message->getStartOffsetPointer();
	const char *end_buffer = request_message->getEndOffsetPointer();

	// clang-format off
	auto increment_buffer_offset = [&start_buffer]
		(size_t inc_size = 1) -> void { start_buffer += inc_size; };
	// clang-format on

	while (start_buffer != end_buffer) {
		switch (current_state) {
		case ParserState::RequestLineBegin:
			if (is_token(*start_buffer)) {
				current_state = ParserState::RequestMethod;
				http_message->pushBackRequestMethod(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestMethod:
			if (*start_buffer == static_cast<char>(LexConsts::SP)) {
				current_state = ParserState::RequestResource;
				increment_buffer_offset();
			} else if (is_token(*start_buffer)) {
				http_message->pushBackRequestMethod(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestResource:
			if (*start_buffer == static_cast<char>(LexConsts::SP)) {
				current_state = ParserState::RequestProtocolH;
				increment_buffer_offset();
			} else if (std::isprint(*start_buffer)) {
				http_message->pushBackTargetResource(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolH:
			if (*start_buffer == 'H') {
				current_state = ParserState::RequestProtocolT1;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolT1:
			if (*start_buffer == 'T') {
				current_state = ParserState::RequestProtocolT2;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolT2:
			if (*start_buffer == 'T') {
				current_state = ParserState::RequestProtocolP;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolP:
			if (*start_buffer == 'P') {
				current_state =
				    ParserState::RequestProtocolSlash;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolSlash:
			if (*start_buffer == '/') {
				current_state =
				    ParserState::RequestProtocolVersionMajor;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolVersionMajor:
			if (std::isdigit(*start_buffer)) {
				current_state = ParserState::RequestProtocolDot;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolDot:
			if (*start_buffer == '.') {
				current_state =
				    ParserState::RequestProtocolVersionMinor;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestProtocolVersionMinor:
			if (std::isdigit(*start_buffer)) {
				http_message->setHTTPVersion(
				    HTTPVersion::HTTP1_1);
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(LexConsts::CR)) {
				current_state = ParserState::RequestLineLF;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::RequestLineLF:
			if (*start_buffer == static_cast<char>(LexConsts::LF)) {
				current_state = ParserState::HeaderName;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::HeaderName:
			if (is_token(*start_buffer)) {
				http_message->pushBackHeaderName(*start_buffer);
				increment_buffer_offset();
			} else if (*start_buffer == ':') {
				current_state = ParserState::HeaderValue;
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(LexConsts::CR)) {
				current_state = ParserState::HeaderEndLF;
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::HeaderValue:
			if (*start_buffer == static_cast<char>(LexConsts::CR)) {
				current_state = ParserState::HeaderValueLF;
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(LexConsts::SP)) {
				if (*(start_buffer - 1) == ':') {
					increment_buffer_offset();
				} else {
					current_state =
					    ParserState::ProtocolError;
				}
			} else if (is_text(*start_buffer)) {
				http_message->pushBackHeaderValue(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		case ParserState::HeaderValueLF:
			if (*start_buffer == static_cast<char>(LexConsts::LF)) {
				increment_buffer_offset();
				http_message->addTempHeadersHolderToMessage();
				current_state = ParserState::HeaderName;
			} else {
				current_state = ParserState::ProtocolError;
			}
			break;
		// clang-format off
		case ParserState::HeaderEndLF:
			if (*start_buffer == static_cast<char>(LexConsts::LF)) {
				if (str3cmp(http_message->getTempRequestMethod().c_str(), "GET")) {
					current_state = ParserState::ParsingDone;
					http_message->setRequestType(HTTPRequestType::Get);
					increment_buffer_offset();
				} else if (str4cmp(http_message->getTempRequestMethod().c_str(), "POST")) {
					current_state = ParserState::MessageBody;
					http_message->setRequestType(HTTPRequestType::Post);
					increment_buffer_offset();
				} else if (str3cmp(http_message->getTempRequestMethod().c_str(), "PUT")) {
					current_state = ParserState::MessageBody;
					http_message->setRequestType(HTTPRequestType::Put);
					increment_buffer_offset();
				} else if (str4cmp(http_message->getTempRequestMethod().c_str(), "HEAD")) {
					current_state = ParserState::ParsingDone;
					http_message->setRequestType(HTTPRequestType::Head);
					increment_buffer_offset();
				} else {
					http_message->setRequestType(HTTPRequestType::Unsupported);
				}
			}
			break;
		// clang-format on
		case ParserState::MessageBody:
			http_message->pushBackRawBody(
			    std::string{start_buffer, end_buffer});
			goto FINISH;
		case ParserState::ParsingDone:
			goto FINISH;
		case ParserState::ProtocolError:
			goto FINISH;
		}
	}
FINISH:
	return std::move(http_message);
}

} // namespace blueth::http
