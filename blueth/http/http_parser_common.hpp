#pragma once
#include "common.hpp"
namespace blueth::http {
#define str3cmp_macro(ptr, c0, c1, c2)                                         \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2

static bool str3cmp(const char *ptr, const char *cmp) {
	return str3cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2));
}
#define str4cmp_macro(ptr, c0, c1, c2, c3)                                     \
	*(ptr + 0) == c0 &&*(ptr + 1) == c1 &&*(ptr + 2) == c2 &&*(ptr + 3) == \
	    c3

static bool str4cmp(const char *ptr, const char *cmp) {
	return str4cmp_macro(ptr, *(cmp + 0), *(cmp + 1), *(cmp + 2),
			     *(cmp + 3));
}

enum class parser_state {
	protocol_error,
	// Request-Line states
	request_line_begin,
	request_method,
	request_resource,
	request_protocol_H,
	request_protocol_T1,
	request_protocol_T2,
	request_protocol_P,
	request_protocol_slash,
	request_protocol_version_major,
	request_protocol_dot,
	request_protocol_version_minor,
	request_line_lf,
	// HTTP 1.x header states
	header_name,
	header_value,
	header_value_lf,
	header_value_end,
	header_end_lf,
	// Request body state
	message_body,
	// Final state, indicates success in parsing
	parsing_done
};

enum class lex_consts { CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09 };

BLUETH_FORCE_INLINE constexpr static bool is_separator(char value) {
	switch (value) {
	case '(':
	case ')':
	case '<':
	case '>':
	case '@':
	case ',':
	case ';':
	case ':':
	case '\\':
	case '"':
	case '/':
	case '[':
	case ']':
	case '?':
	case '=':
	case '{':
	case '}':
	case static_cast<char>(lex_consts::SP):
	case static_cast<char>(lex_consts::HT):
		return true;
	default:
		return false;
	}
}

BLUETH_FORCE_INLINE constexpr static bool is_char(char value) {
	return (static_cast<unsigned>(value) <= 127);
}

BLUETH_FORCE_INLINE constexpr static bool is_control(char value) {
	return ((value >= 0 && value <= 31) || value == 127);
}

BLUETH_FORCE_INLINE constexpr static bool is_token(char value) {
	return (is_char(value) && !(is_control(value) || is_separator(value)));
}

BLUETH_FORCE_INLINE constexpr static bool is_text(char value) {
	return (!is_control(value) ||
		(value == static_cast<char>(lex_consts::SP) ||
		 (value == static_cast<char>(lex_consts::HT))));
}

} // namespace blueth::http
