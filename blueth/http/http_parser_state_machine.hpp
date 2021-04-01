#pragma once
#include "common.hpp"
#include "http/http_constants.hpp"
#include "http_message.hpp"
#include "http_parser_common.hpp"
#include "io/IOBuffer.hpp"
#include <cctype>
#include <memory>
#include <utility>

#include <iostream>
#define print_me std::cout << "Here" << std::endl;
namespace blueth::http {

inline std::pair<parser_state, std::unique_ptr<http_request_message>>
parse_http_1_1_request_message(
    const std::unique_ptr<io::IOBuffer<char>> &request_message,
    parser_state &current_state,
    std::unique_ptr<http_request_message> http_message) {
	const char *start_buffer = request_message->getStartOffsetPointer();
	const char *end_buffer = request_message->getEndOffsetPointer();

	// clang-format off
	auto increment_buffer_offset = [&start_buffer]
		(size_t inc_size = 1) -> void { start_buffer += inc_size; };
	// clang-format on

	while (start_buffer != end_buffer) {
		switch (current_state) {
		case parser_state::request_line_begin:
			if (is_token(*start_buffer)) {
				current_state = parser_state::request_method;
				http_message->push_back_request_method(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_method:
			if (*start_buffer ==
			    static_cast<char>(lex_consts::SP)) {
				current_state = parser_state::request_resource;
				increment_buffer_offset();
			} else if (is_token(*start_buffer)) {
				http_message->push_back_request_method(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_resource:
			if (*start_buffer ==
			    static_cast<char>(lex_consts::SP)) {
				current_state =
				    parser_state::request_protocol_H;
				increment_buffer_offset();
			} else if (std::isprint(*start_buffer)) {
				http_message->push_back_target_resource(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_H:
			if (*start_buffer == 'H') {
				current_state =
				    parser_state::request_protocol_T1;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_T1:
			if (*start_buffer == 'T') {
				current_state =
				    parser_state::request_protocol_T2;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_T2:
			if (*start_buffer == 'T') {
				current_state =
				    parser_state::request_protocol_P;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_P:
			if (*start_buffer == 'P') {
				current_state =
				    parser_state::request_protocol_slash;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_slash:
			if (*start_buffer == '/') {
				current_state = parser_state::
				    request_protocol_version_major;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_version_major:
			if (std::isdigit(*start_buffer)) {
				current_state =
				    parser_state::request_protocol_dot;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_dot:
			if (*start_buffer == '.') {
				current_state = parser_state::
				    request_protocol_version_minor;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_protocol_version_minor:
			if (std::isdigit(*start_buffer)) {
				http_message->set_http_version(
				    http_version::http_1_1);
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(lex_consts::CR)) {
				current_state = parser_state::request_line_lf;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::request_line_lf:
			if (*start_buffer ==
			    static_cast<char>(lex_consts::LF)) {
				current_state = parser_state::header_name;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::header_name:
			if (is_token(*start_buffer)) {
				http_message->push_back_header_name(
				    *start_buffer);
				increment_buffer_offset();
			} else if (*start_buffer == ':') {
				current_state = parser_state::header_value;
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(lex_consts::CR)) {
				current_state = parser_state::header_end_lf;
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::header_value:
			if (*start_buffer ==
			    static_cast<char>(lex_consts::CR)) {
				current_state = parser_state::header_value_lf;
				increment_buffer_offset();
			} else if (*start_buffer ==
				   static_cast<char>(lex_consts::SP)) {
				if (*(start_buffer - 1) == ':') {
					increment_buffer_offset();
				} else {
					current_state =
					    parser_state::protocol_error;
				}
			} else if (is_text(*start_buffer)) {
				http_message->push_back_header_value(
				    *start_buffer);
				increment_buffer_offset();
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		case parser_state::header_value_lf:
			if (*start_buffer ==
			    static_cast<char>(lex_consts::LF)) {
				increment_buffer_offset();
				http_message->add_temp_header_holder_to_message();
				current_state = parser_state::header_name;
			} else {
				current_state = parser_state::protocol_error;
			}
			break;
		// clang-format off
		case parser_state::header_end_lf:
			if(*start_buffer == static_cast<char>(lex_consts::LF)) {
				if (str3cmp(http_message->get_temp_request_method().c_str(), "GET")) {
					current_state = parser_state::parsing_done;
					http_message->set_request_type(http_request_type::get);
					increment_buffer_offset();
				} else if (str4cmp(http_message->get_temp_request_method().c_str(), "POST")) {
					current_state = parser_state::message_body;
					http_message->set_request_type(http_request_type::post);
					increment_buffer_offset();
				} else if (str3cmp(http_message->get_temp_request_method().c_str(), "PUT")) {
					current_state = parser_state::message_body;
					http_message->set_request_type(http_request_type::put);
					increment_buffer_offset();
				} else if (str4cmp(http_message->get_temp_request_method().c_str(), "HEAD")) {
					current_state = parser_state::parsing_done;
					http_message->set_request_type(http_request_type::head);
					increment_buffer_offset();
				} else {
					http_message->set_request_type(http_request_type::unsupported);
				}
			}
			break;
		// clang-format on
		case parser_state::message_body:
			http_message->push_back_raw_body(
			    std::string{start_buffer, end_buffer});
			goto FINISH;
		case parser_state::parsing_done:
			goto FINISH;
		case parser_state::protocol_error:
			goto FINISH;
		}
	}
FINISH:
	return {current_state, std::move(http_message)};
}

inline std::string state_as_string(parser_state state) noexcept {
	switch(state){
		case parser_state::protocol_error:
			return "protocol_error";
		case parser_state::request_line_begin:
			return "request_line_begin";
		case parser_state::request_method:
			return "request_method";
		case parser_state::request_resource:
			return "request_resource";
		case parser_state::request_protocol_H:
			return "request_protocol_H";
		case parser_state::request_protocol_T1:
			return "request_protocol_T1";
		case parser_state::request_protocol_T2:
			return "request_protocol_T2";
		case parser_state::request_protocol_P:
			return "request_protocol_P";
		case parser_state::request_protocol_slash:
			return "request_protocol_slash";
		case parser_state::request_protocol_version_major:
			return "request_protocol_version_major";
		case parser_state::request_protocol_dot:
			return "request_protocol_dot";
		case parser_state::request_protocol_version_minor:
			return "request_protocol_version_minor";
		case parser_state::request_line_lf:
			return "request_line_lf";
		case parser_state::header_name:
			return "header_name";
		case parser_state::header_value:
			return "header_value";
		case parser_state::header_value_lf:
			return "header_value_lf";
		case parser_state::header_end_lf:
			return "header_end_lf";
		case parser_state::message_body:
			return "message_body";
		case parser_state::parsing_done:
			return "parsing_done";
	}
}

} // namespace blueth::http
