#pragma once
#include "HTTPMessage.hpp"
#include "HTTPParserCommon.hpp"
#include "common.hpp"
#include "http/HTTPConstants.hpp"
#include "io/IOBuffer.hpp"
#include <cctype>
#include <cstdlib>
#include <memory>

#include <iostream>
#define print_me(x) std::cout << x << std::endl;
namespace blueth::http {

inline std::unique_ptr<HTTPResponseMessage> ParseHTTP1_1ResponseMessage(
    const std::unique_ptr<io::IOBuffer<char>> &response_message,
    ResponseParserState &current_state,
    std::unique_ptr<HTTPResponseMessage> http_message) {
	const char *start_buffer = response_message->getStartOffsetPointer();
	const char *end_buffer = response_message->getEndOffsetPointer();

	// clang-format off
		auto increment_buffer_offset = [&start_buffer]
			(size_t inc_size = 1) -> void { start_buffer += inc_size; };
	// clang-format on

	while (start_buffer != end_buffer) {
		switch (current_state) {
		case ResponseParserState::ResponseProtocolH:
			if (*start_buffer == 'H') {
				current_state =
				    ResponseParserState::ResponseProtocolT1;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolT1:
			if (*start_buffer == 'T') {
				current_state =
				    ResponseParserState::ResponseProtocolT2;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolT2:
			if (*start_buffer == 'T') {
				current_state =
				    ResponseParserState::ResponseProtocolP;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolP:
			if (*start_buffer == 'P') {
				current_state =
				    ResponseParserState::ResponseProtocolSlash;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolSlash:
			if (*start_buffer == '/') {
				current_state = ResponseParserState::
				    ResponseProtocolVersionMajor;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolVersionMajor:
			if (*start_buffer == '1') {
				current_state =
				    ResponseParserState::ResponseProtocolDot;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolDot:
			if (*start_buffer == '.') {
				current_state = ResponseParserState::
				    ResponseProtocolVersionMinor;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseProtocolVersionMinor:
			if (std::isdigit(*start_buffer)) {
				http_message->setHTTPVersion(
				    HTTPVersion::HTTP1_1);
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(LexConsts::SP)) {
				current_state = ResponseParserState::StatusCode;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::StatusCode:
			if (std::isdigit(*start_buffer)) {
				http_message->pushBackResponseCode(
				    *start_buffer);
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(LexConsts::SP)) {
				http_message->setResponseCode(
				    static_cast<HTTPResponseCodes>(std::atoi(
					http_message->getTempStatusCode())));
				current_state =
				    ResponseParserState::ResponseReasonPhrase;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseReasonPhrase:
			if (*start_buffer == static_cast<char>(LexConsts::CR)) {
				current_state =
				    ResponseParserState::StatusLineLF;
				increment_buffer_offset();
			} else if (is_text(*start_buffer)) {
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::StatusLineLF:
			if (*start_buffer == static_cast<char>(LexConsts::LF)) {
				current_state = ResponseParserState::HeaderName;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::HeaderName:
			if (is_token(*start_buffer)) {
				http_message->pushBackHeaderName(*start_buffer);
				increment_buffer_offset();
			} else if (*start_buffer == ':') {
				current_state =
				    ResponseParserState::HeaderValue;
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(LexConsts::CR)) {
				current_state =
				    ResponseParserState::HeaderEndLF;
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::HeaderValue:
			if (*start_buffer == static_cast<char>(LexConsts::CR)) {
				current_state =
				    ResponseParserState::HeaderValueLF;
				increment_buffer_offset();
			} else if (is_text(*start_buffer)) {
				if (*(start_buffer - 1) == ':') {
					increment_buffer_offset();
					break;
				}
				http_message->pushBackHeaderValue(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::HeaderValueLF:
			if (*start_buffer == static_cast<char>(LexConsts::LF)) {
				increment_buffer_offset();
				http_message->addTempHeadersHolderToMessage();
				current_state = ResponseParserState::HeaderName;
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::HeaderEndLF:
			if (*start_buffer == static_cast<char>(LexConsts::LF)) {
				if (http_message->constGetHTTPHeaders()
					->headerContains("Content-Length")) {
					current_state = ResponseParserState::
					    ResponseMessageBody;
				} else {
					current_state =
					    ResponseParserState::ParsingDone;
				}
				increment_buffer_offset();
			} else {
				current_state =
				    ResponseParserState::ProtocolError;
			}
			break;
		case ResponseParserState::ResponseMessageBody:
			http_message->pushBackRawBody(
			    std::string{start_buffer, end_buffer});
			current_state = ResponseParserState::ParsingDone;
			goto FINISH;
		case ResponseParserState::ParsingDone:
			goto FINISH;
		case ResponseParserState::ProtocolError:
			goto FINISH;
		}
	}
FINISH:
	return std::move(http_message);
}

} // namespace blueth::http
