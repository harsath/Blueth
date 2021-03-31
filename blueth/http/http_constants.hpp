#pragma once
namespace blueth::http {

enum class http_response_code {
	ok = 200,
	bad_request = 400,
	not_found = 400,
	forbidden = 403,
	not_acceptable = 406,
	method_not_allowed = 405,
	unsupported_media_type = 415,
	created = 201,
	moved_permanently = 301,
	unauthorized = 401
};

enum class http_request_type { get, post, head, put, unsupported };

enum class http_server_type { plaintext_server, ssl_server };

enum class stream_type {
	ssl_sync_stream,
	ssl_async_stream,
	sync_stream,
	async_stream
};

enum class http_version { http_1_1, http_2 };

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

} // namespace blueth::http
