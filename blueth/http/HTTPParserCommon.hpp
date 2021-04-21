#pragma once
#include "HTTPConstants.hpp"
#include "common.hpp"
#include <optional>
#include <string>

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

static std::optional<std::string>
string_from_response_code(HTTPResponseCodes response_code) {
	switch (response_code) {
	case HTTPResponseCodes::Continue:
		return "Continue";
	case HTTPResponseCodes::SwitchingProtocols:
		return "Switching Protocols";
	case HTTPResponseCodes::Ok:
		return "OK";
	case HTTPResponseCodes::Created:
		return "Created";
	case HTTPResponseCodes::Accepted:
		return "Accepted";
	case HTTPResponseCodes::NonAuthoritativeInformation:
		return "Non-Authoritative Information";
	case HTTPResponseCodes::NoContent:
		return "No Content";
	case HTTPResponseCodes::ResetContent:
		return "Reset Content";
	case HTTPResponseCodes::PartialContent:
		return "Partial Content";
	case HTTPResponseCodes::MultipleChoices:
		return "Multiple Choices";
	case HTTPResponseCodes::MovedPermanently:
		return "Moved Permanently";
	case HTTPResponseCodes::Found:
		return "Found";
	case HTTPResponseCodes::SeeOther:
		return "See Other";
	case HTTPResponseCodes::NotModified:
		return "Not Modified";
	case HTTPResponseCodes::UseProxy:
		return "Use Proxy";
	case HTTPResponseCodes::TemporaryRedirect:
		return "Temporary Redirect";
	case HTTPResponseCodes::BadRequest:
		return "Bad Request";
	case HTTPResponseCodes::Unauthorized:
		return "Unauthorized";
	case HTTPResponseCodes::PaymentRequired:
		return "Payment Required";
	case HTTPResponseCodes::Forbidden:
		return "Forbidden";
	case HTTPResponseCodes::NotFound:
		return "Not Found";
	case HTTPResponseCodes::MethodNotAllowed:
		return "Method Not Allowed";
	case HTTPResponseCodes::NotAcceptable:
		return "Not Acceptable";
	case HTTPResponseCodes::ProxyAuthenticationRequired:
		return "Proxy Authentication Required";
	case HTTPResponseCodes::RequestTimeOut:
		return "Request Time-out";
	case HTTPResponseCodes::Conflict:
		return "Conflict";
	case HTTPResponseCodes::Gone:
		return "Gone";
	case HTTPResponseCodes::LengthRequired:
		return "Length Required";
	case HTTPResponseCodes::PreconditionFailed:
		return "Precondition Failed";
	case HTTPResponseCodes::RequestEntityTooLarge:
		return "Request Entity Too Large";
	case HTTPResponseCodes::RequestURITooLarge:
		return "Request-URI Too Large";
	case HTTPResponseCodes::UnsupportedMediaType:
		return "Unsupported Media Type";
	case HTTPResponseCodes::RequestRangeNotSatisfiable:
		return "Requested range not satisfiable";
	case HTTPResponseCodes::ExpectationFailed:
		return "Expectation Failed";
	case HTTPResponseCodes::InternalServerError:
		return "Internal Server Error";
	case HTTPResponseCodes::NotImplemented:
		return "Not Implemented";
	case HTTPResponseCodes::BadGateway:
		return "Bad Gateway";
	case HTTPResponseCodes::ServiceUnavailable:
		return "Service Unavailable";
	case HTTPResponseCodes::GatewayTimeOut:
		return "Gateway Time-out";
	case HTTPResponseCodes::HttpVersionNotSupported:
		return "HTTP Version not supported";
	case HTTPResponseCodes::InvalidHttpCode:
		return std::nullopt;
	default:
		return std::nullopt;
	}
}

} // namespace blueth::http
