#pragma once
namespace blueth::http {

enum class HTTPResponseCodes {
	Ok = 200,
	BadRequest = 400,
	NotFound = 400,
	Forbidden = 403,
	NotAcceptable = 406,
	MethodNotAllowed = 405,
	UnsupportedMediaType = 415,
	Created = 201,
	MovedPermanently = 301,
	Unauthorized = 401
};

enum class HTTPRequestType { Get, Post, Head, Put, Unsupported };

enum class HTTPServerType { PlaintextServer, SSLServer };

enum class StreamType {
	SSLSyncStream,
	SSLAsyncStream,
	SyncStream,
	AsyncStream
};

enum class HTTPVersion { HTTP1_1, HTTP_2 };

enum class ParserState {
	ProtocolError,
	// Request-Line states
	RequestLineBegin,
	RequestMethod,
	RequestResource,
	RequestProtocolH,
	RequestProtocolT1,
	RequestProtocolT2,
	RequestProtocolP,
	RequestProtocolSlash,
	RequestProtocolVersionMajor,
	RequestProtocolDot,
	RequestProtocolVersionMinor,
	RequestLineLF,
	// HTTP 1.x header states
	HeaderName,
	HeaderValue,
	HeaderValueLF,
	HeaderEndLF,
	// Request body state
	MessageBody,
	// Final state, indicates success in parsing
	ParsingDone
};

enum class LexConsts { CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09 };

} // namespace blueth::http
