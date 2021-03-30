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
} // namespace blueth::http
