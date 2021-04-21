#pragma once
namespace blueth::http {

enum class HTTPResponseCodes {
	Continue = 100,
	SwitchingProtocols = 101,
	Ok = 200,
	Created = 201,
	Accepted = 202,
	NonAuthoritativeInformation = 203,
	NoContent = 204,
	ResetContent = 205,
	PartialContent = 206,
	MultipleChoices = 300,
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
	NotModified = 304,
	UseProxy = 305,
	TemporaryRedirect = 307,
	BadRequest = 400,
	Unauthorized = 401,
	PaymentRequired = 402,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	NotAcceptable = 406,
	ProxyAuthenticationRequired = 407,
	RequestTimeOut = 408,
	Conflict = 409,
	Gone = 410,
	LengthRequired = 411,
	PreconditionFailed = 412,
	RequestEntityTooLarge = 413,
	RequestURITooLarge = 414,
	UnsupportedMediaType = 415,
	RequestRangeNotSatisfiable = 416,
	ExpectationFailed = 417,
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeOut = 504,
	HttpVersionNotSupported = 505,
	InvalidHttpCode = 506
};

enum class HTTPRequestType { Get, Post, Head, Put, Unsupported, Connect };

enum class HTTPServerType { PlaintextServer, SSLServer };

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

enum class ResponseParserState {
	ProtocolError,
	// Status-line states
	ResponseProtocolH,
	ResponseProtocolT1,
	ResponseProtocolT2,
	ResponseProtocolP,
	ResponseProtocolSlash,
	ResponseProtocolVersionMajor,
	ResponseProtocolDot,
	ResponseProtocolVersionMinor,
	StatusCode,
	ResponseReasonPhrase,
	StatusLineLF,
	// HTTP 1.x header states
	HeaderName,
	HeaderValue,
	HeaderValueLF,
	HeaderEndLF,
	// Resonse Body state
	ResponseMessageBody,
	// Final state, indicates success in parsing
	ParsingDone
};

enum class LexConsts { CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09 };

} // namespace blueth::http
