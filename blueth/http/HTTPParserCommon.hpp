#pragma once
#include "HTTPConstants.hpp"
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
	case static_cast<char>(LexConsts::SP):
	case static_cast<char>(LexConsts::HT):
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
		(value == static_cast<char>(LexConsts::SP) ||
		 (value == static_cast<char>(LexConsts::HT))));
}

} // namespace blueth::http
