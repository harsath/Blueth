#pragma once
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPParserCommon.hpp"
#include "common.hpp"
#include "io/IOBuffer.hpp"
#include <algorithm>
#include <memory>

namespace blueth::http {

class HTTPRequestMessage {
      private:
	static constexpr std::size_t initial_capacity_ = 2048;
	std::unique_ptr<HTTPHeaders> http_headers_{nullptr};
	std::unique_ptr<io::IOBuffer<char>> raw_body_{nullptr};
	HTTPRequestType request_type_;
	HTTPVersion http_message_version_;
	std::string target_resource_;

	// These are only to work with the state-machine parser
	std::string temp_header_value_holder_;
	std::string temp_header_name_holder_;
	std::string temp_http_method_holder_;

      public:
	HTTPRequestMessage();
	BLUETH_FORCE_INLINE static std::unique_ptr<HTTPRequestMessage> create();
	template <typename T1, typename T2>
	BLUETH_FORCE_INLINE void addHeader(T1 &&header_name,
					   T2 &&header_value) noexcept;
	template <typename T>
	BLUETH_FORCE_INLINE bool removeHeader(T &&header_name) noexcept;
	BLUETH_FORCE_INLINE const std::unique_ptr<HTTPHeaders> &
	constGetHTTPHeaders() const noexcept;
	BLUETH_NODISCARD BLUETH_FORCE_INLINE std::unique_ptr<HTTPHeaders>
	getHTTPHeaders() noexcept;
	BLUETH_FORCE_INLINE void flushBody() noexcept;
	BLUETH_FORCE_INLINE void setRequestType(HTTPRequestType type) noexcept;
	BLUETH_FORCE_INLINE HTTPRequestType getRequestType() const noexcept;
	BLUETH_FORCE_INLINE void setHTTPVersion(HTTPVersion version) noexcept;
	BLUETH_FORCE_INLINE HTTPVersion getHTTPVersion() const noexcept;
	template <typename T>
	BLUETH_FORCE_INLINE void
	setHTTPTargetResource(T &&target_resource) noexcept;
	BLUETH_FORCE_INLINE const std::string &
	getTargetResource() const noexcept;
	void setRawBody(std::unique_ptr<io::IOBuffer<char>> io_buffer) noexcept;
	template <typename T>
	BLUETH_FORCE_INLINE std::optional<std::string>
	getHeaderValue(T &&header_name) noexcept;
	std::string buildRawMessage() const noexcept;

	// These methods are only implemented to work with the state-machine
	// parser
	BLUETH_FORCE_INLINE void pushBackHeaderValue(char char_val) noexcept;
	BLUETH_FORCE_INLINE void pushBackHeaderName(char char_val) noexcept;
	BLUETH_FORCE_INLINE void addTempHeadersHolderToMessage() noexcept;
	BLUETH_FORCE_INLINE void
	pushBackRawBody(std::string &&raw_body) noexcept;
	BLUETH_FORCE_INLINE void pushBackRawBody(char char_val) noexcept;
	BLUETH_FORCE_INLINE void pushBackTargetResource(char char_val) noexcept;
	BLUETH_FORCE_INLINE void pushBackRequestMethod(char char_val) noexcept;
	BLUETH_FORCE_INLINE std::string getTempRequestMethod() const noexcept;
};

class HTTPResponseMessage {
      private:
	static constexpr std::size_t initial_capacity_ = 2048;
	std::unique_ptr<HTTPHeaders> http_headers_{nullptr};
	std::unique_ptr<io::IOBuffer<char>> raw_body_{nullptr};
	HTTPResponseCodes response_code_;
	HTTPVersion http_message_version_;

	// These are only to work with the state-machine parser for incremental
	// parsing
	std::string temp_header_value_holder_;
	std::string temp_header_name_holder_;
	struct {
		char http_code_holder[3];
		size_t current_index{};
	} temp_http_status_code_holder_;

      public:
	HTTPResponseMessage();
	BLUETH_FORCE_INLINE static std::unique_ptr<HTTPResponseMessage>
	create();
	template <typename T1, typename T2>
	BLUETH_FORCE_INLINE void addHeader(T1 &&header_name,
					   T2 &&header_value) noexcept;
	template <typename T>
	BLUETH_FORCE_INLINE bool removeHeader(T &&header_name) noexcept;
	BLUETH_FORCE_INLINE const std::unique_ptr<HTTPHeaders> &
	constGetHTTPHeaders() const noexcept;
	BLUETH_NODISCARD BLUETH_FORCE_INLINE std::unique_ptr<HTTPHeaders>
	getHTTPHeaders() noexcept;
	BLUETH_FORCE_INLINE void flushBody() noexcept;
	BLUETH_FORCE_INLINE void
	setResponseCode(HTTPResponseCodes code) noexcept;
	BLUETH_FORCE_INLINE HTTPResponseCodes getResponseCode() const noexcept;
	BLUETH_FORCE_INLINE void setHTTPVersion(HTTPVersion version) noexcept;
	BLUETH_FORCE_INLINE HTTPVersion getHTTPVersion() const noexcept;
	void setRawBody(std::unique_ptr<io::IOBuffer<char>> io_buffer) noexcept;
	template <typename T>
	BLUETH_FORCE_INLINE std::optional<std::string>
	getHeaderValue(T &&header_name) noexcept;
	std::string buildRawMessage() const noexcept;

	// These methods are only implemented to work with the incremental
	// state-machine parser. These methods must not be called by a user of
	// the class other than the parser.
	BLUETH_FORCE_INLINE void pushBackHeaderValue(char char_val) noexcept;
	BLUETH_FORCE_INLINE void pushBackHeaderName(char char_val) noexcept;
	BLUETH_FORCE_INLINE void addTempHeadersHolderToMessage() noexcept;
	BLUETH_FORCE_INLINE void
	pushBackRawBody(std::string &&raw_body) noexcept;
	BLUETH_FORCE_INLINE void pushBackResponseCode(char char_value) noexcept;
	BLUETH_FORCE_INLINE const char *getTempStatusCode() const noexcept;
};

BLUETH_FORCE_INLINE inline std::unique_ptr<HTTPRequestMessage>
HTTPRequestMessage::create() {
	return std::make_unique<HTTPRequestMessage>();
}

inline HTTPRequestMessage::HTTPRequestMessage()
    : request_type_{HTTPRequestType::Unsupported},
      http_message_version_{HTTPVersion::HTTP1_1},
      temp_header_value_holder_{""}, temp_header_name_holder_{""},
      target_resource_{""}, http_headers_{std::make_unique<HTTPHeaders>()},
      raw_body_{io::IOBuffer<char>::create(initial_capacity_)} {}

template <typename T1, typename T2>
BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::addHeader(T1 &&header_name, T2 &&header_value) noexcept {
	http_headers_->addHeader(
	    {std::forward<T1>(header_name), std::forward<T2>(header_value)});
}

template <typename T>
BLUETH_FORCE_INLINE inline bool
HTTPRequestMessage::removeHeader(T &&header_name) noexcept {
	return http_headers_->removeHeader(std::forward<T>(header_name));
}

BLUETH_FORCE_INLINE inline const std::unique_ptr<HTTPHeaders> &
HTTPRequestMessage::constGetHTTPHeaders() const noexcept {
	return http_headers_;
}

BLUETH_NODISCARD BLUETH_FORCE_INLINE inline std::unique_ptr<HTTPHeaders>
HTTPRequestMessage::getHTTPHeaders() noexcept {
	return std::move(http_headers_);
}

BLUETH_FORCE_INLINE inline void HTTPRequestMessage::flushBody() noexcept {
	raw_body_->clear();
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::setRequestType(HTTPRequestType req_type) noexcept {
	request_type_ = req_type;
}

BLUETH_FORCE_INLINE inline HTTPRequestType
HTTPRequestMessage::getRequestType() const noexcept {
	return request_type_;
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::setHTTPVersion(HTTPVersion version) noexcept {
	http_message_version_ = version;
}

BLUETH_FORCE_INLINE inline HTTPVersion
HTTPRequestMessage::getHTTPVersion() const noexcept {
	return http_message_version_;
}

template <typename T>
BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::setHTTPTargetResource(T &&target_resource) noexcept {
	target_resource_ += std::forward<T>(target_resource);
}

BLUETH_FORCE_INLINE inline const std::string &
HTTPRequestMessage::getTargetResource() const noexcept {
	return target_resource_;
}

inline void HTTPRequestMessage::setRawBody(
    std::unique_ptr<io::IOBuffer<char>> io_buffer) noexcept {
	raw_body_ = std::move(io_buffer);
}

template <typename T>
BLUETH_FORCE_INLINE inline std::optional<std::string>
HTTPRequestMessage::getHeaderValue(T &&header_name) noexcept {
	return http_headers_->getHeaderValue(std::forward<T>(header_name));
}

inline std::string HTTPRequestMessage::buildRawMessage() const noexcept {
	std::string returner;
	switch (request_type_) {
	case HTTPRequestType::Get:
		returner += "GET ";
		break;
	case HTTPRequestType::Post:
		returner += "POST ";
		break;
	case HTTPRequestType::Head:
		returner += "HEAD ";
		break;
	case HTTPRequestType::Put:
		returner += "PUT ";
		break;
	case HTTPRequestType::Connect:
		returner += "CONNECT ";
		break;
	case HTTPRequestType::Unsupported:
		returner += "UNSUPPORTED ";
		break;
	}
	returner += target_resource_;
	returner += " ";
	returner += "HTTP/1.1";
	returner += "\r\n";
	returner += http_headers_->buildRawHeader();
	returner += std::string{raw_body_->begin(), raw_body_->end()};
	return returner;
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::pushBackHeaderValue(char char_val) noexcept {
	temp_header_value_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::pushBackHeaderName(char char_val) noexcept {
	temp_header_name_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::addTempHeadersHolderToMessage() noexcept {
	http_headers_->addHeader({std::move(temp_header_name_holder_),
				  std::move(temp_header_value_holder_)});
	temp_header_name_holder_.clear();
	temp_header_value_holder_.clear();
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::pushBackRawBody(std::string &&raw_body) noexcept {
	raw_body_->appendRawBytes(raw_body.c_str(), raw_body.size());
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::pushBackRawBody(char char_val) noexcept {
	raw_body_->appendRawBytes(&char_val, 1);
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::pushBackTargetResource(char char_val) noexcept {
	target_resource_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline void
HTTPRequestMessage::pushBackRequestMethod(char char_val) noexcept {
	temp_http_method_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline std::string
HTTPRequestMessage::getTempRequestMethod() const noexcept {
	return temp_http_method_holder_;
}

BLUETH_FORCE_INLINE inline std::unique_ptr<HTTPResponseMessage>
HTTPResponseMessage::create() {
	return std::make_unique<HTTPResponseMessage>();
}

template <typename T1, typename T2>
BLUETH_FORCE_INLINE inline void
HTTPResponseMessage::addHeader(T1 &&header_name, T2 &&header_value) noexcept {
	http_headers_->addHeader(
	    {std::forward<T1>(header_name), std::forward<T2>(header_value)});
}

template <typename T>
BLUETH_FORCE_INLINE inline bool
HTTPResponseMessage::removeHeader(T &&header_name) noexcept {
	return http_headers_->removeHeader(std::forward<T>(header_name));
}

BLUETH_FORCE_INLINE inline const std::unique_ptr<HTTPHeaders> &
HTTPResponseMessage::constGetHTTPHeaders() const noexcept {
	return http_headers_;
}

BLUETH_NODISCARD BLUETH_FORCE_INLINE inline std::unique_ptr<HTTPHeaders>
HTTPResponseMessage::getHTTPHeaders() noexcept {
	return std::move(http_headers_);
}

BLUETH_FORCE_INLINE inline void HTTPResponseMessage::flushBody() noexcept {
	raw_body_->clear();
}

BLUETH_FORCE_INLINE inline void
HTTPResponseMessage::setResponseCode(HTTPResponseCodes code) noexcept {
	response_code_ = code;
}

BLUETH_FORCE_INLINE inline HTTPResponseCodes
HTTPResponseMessage::getResponseCode() const noexcept {
	return response_code_;
}

BLUETH_FORCE_INLINE inline void
HTTPResponseMessage::setHTTPVersion(HTTPVersion version) noexcept {
	http_message_version_ = version;
}

BLUETH_FORCE_INLINE inline HTTPVersion
HTTPResponseMessage::getHTTPVersion() const noexcept {
	return http_message_version_;
}

inline void HTTPResponseMessage::setRawBody(
    std::unique_ptr<io::IOBuffer<char>> io_buffer) noexcept {
	raw_body_ = std::move(io_buffer);
}

template <typename T>
BLUETH_FORCE_INLINE inline std::optional<std::string>
HTTPResponseMessage::getHeaderValue(T &&header_name) noexcept {
	return http_headers_->getHeaderValue(std::forward<T>(header_name));
}

inline std::string HTTPResponseMessage::buildRawMessage() const noexcept {
	std::string returner;
	// Only supports HTTP/1.x message as of now
	if (http_message_version_ == HTTPVersion::HTTP1_1) {
		returner += "HTTP/1.1";
	} else {
		// TODO(me): HTTP message template
		return "";
	}
	switch (response_code_) {
	case HTTPResponseCodes::Ok:
		returner +=
		    std::to_string(static_cast<int>(HTTPResponseCodes::Ok));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Ok).value();
		break;
	case HTTPResponseCodes::NotFound:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::NotFound));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::NotFound)
			.value();
		break;
	case HTTPResponseCodes::BadRequest:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::BadRequest));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::BadRequest)
			.value();
		break;
	case HTTPResponseCodes::Unauthorized:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::Unauthorized));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Unauthorized)
			.value();
		break;
	case HTTPResponseCodes::Created:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::Created));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Created)
			.value();
		break;
	case HTTPResponseCodes::Found:
		returner +=
		    std::to_string(static_cast<int>(HTTPResponseCodes::Found));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Found).value();
		break;
	case HTTPResponseCodes::Continue:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::Continue));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Continue)
			.value();
		break;
	case HTTPResponseCodes::Accepted:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::Accepted));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Accepted)
			.value();
		break;
	case HTTPResponseCodes::NonAuthoritativeInformation:
		returner += std::to_string(static_cast<int>(
		    HTTPResponseCodes::NonAuthoritativeInformation));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::NonAuthoritativeInformation)
				.value();
		break;
	case HTTPResponseCodes::NoContent:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::NoContent));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::NoContent)
			.value();
		break;
	case HTTPResponseCodes::ResetContent:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::ResetContent));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::ResetContent)
			.value();
		break;
	case HTTPResponseCodes::PartialContent:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::PartialContent));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::PartialContent)
			.value();
		break;
	case HTTPResponseCodes::MultipleChoices:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::MultipleChoices));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::MultipleChoices)
				.value();
		break;
	case HTTPResponseCodes::SeeOther:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::SeeOther));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::SeeOther)
			.value();
		break;
	case HTTPResponseCodes::NotModified:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::NotModified));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::NotModified)
			.value();
		break;
	case HTTPResponseCodes::UseProxy:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::UseProxy));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::UseProxy)
			.value();
		break;
	case HTTPResponseCodes::TemporaryRedirect:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::TemporaryRedirect));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::UseProxy)
			.value();
		break;
	case HTTPResponseCodes::PaymentRequired:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::PaymentRequired));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::PaymentRequired)
				.value();
		break;
	case HTTPResponseCodes::Forbidden:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::Forbidden));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Forbidden)
			.value();
		break;
	case HTTPResponseCodes::MethodNotAllowed:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::MethodNotAllowed));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::MethodNotAllowed)
				.value();
		break;
	case HTTPResponseCodes::NotAcceptable:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::NotAcceptable));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::NotAcceptable)
			.value();
		break;
	case HTTPResponseCodes::ProxyAuthenticationRequired:
		returner += std::to_string(static_cast<int>(
		    HTTPResponseCodes::ProxyAuthenticationRequired));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::ProxyAuthenticationRequired)
				.value();
		break;
	case HTTPResponseCodes::RequestTimeOut:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::RequestTimeOut));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::RequestTimeOut)
			.value();
		break;
	case HTTPResponseCodes::Conflict:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::Conflict));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Conflict)
			.value();
		break;
	case HTTPResponseCodes::Gone:
		returner +=
		    std::to_string(static_cast<int>(HTTPResponseCodes::Gone));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::Gone).value();
		break;
	case HTTPResponseCodes::LengthRequired:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::LengthRequired));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::LengthRequired)
			.value();
		break;
	case HTTPResponseCodes::PreconditionFailed:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::PreconditionFailed));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::PreconditionFailed)
				.value();
		break;
	case HTTPResponseCodes::RequestEntityTooLarge:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::RequestEntityTooLarge));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::RequestEntityTooLarge)
				.value();
		break;
	case HTTPResponseCodes::RequestURITooLarge:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::RequestURITooLarge));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::RequestURITooLarge)
				.value();
		break;
	case HTTPResponseCodes::RequestRangeNotSatisfiable:
		returner += std::to_string(static_cast<int>(
		    HTTPResponseCodes::RequestRangeNotSatisfiable));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::RequestRangeNotSatisfiable)
				.value();
		break;
	case HTTPResponseCodes::ExpectationFailed:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::ExpectationFailed));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::ExpectationFailed)
				.value();
		break;
	case HTTPResponseCodes::InternalServerError:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::InternalServerError));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::InternalServerError)
				.value();
		break;
	case HTTPResponseCodes::NotImplemented:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::NotImplemented));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::NotImplemented)
			.value();
		break;
	case HTTPResponseCodes::BadGateway:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::BadGateway));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::BadGateway)
			.value();
		break;
	case HTTPResponseCodes::ServiceUnavailable:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::ServiceUnavailable));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::ServiceUnavailable)
				.value();
		break;
	case HTTPResponseCodes::GatewayTimeOut:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::GatewayTimeOut));
		returner += " ";
		returner +=
		    string_from_response_code(HTTPResponseCodes::GatewayTimeOut)
			.value();
		break;
	case HTTPResponseCodes::HttpVersionNotSupported:
		returner += std::to_string(static_cast<int>(
		    HTTPResponseCodes::HttpVersionNotSupported));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::HttpVersionNotSupported)
				.value();
		break;

	case HTTPResponseCodes::SwitchingProtocols:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::SwitchingProtocols));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::SwitchingProtocols)
				.value();
		break;

	case HTTPResponseCodes::UnsupportedMediaType:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::UnsupportedMediaType));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::UnsupportedMediaType)
				.value();
		break;
	case HTTPResponseCodes::MovedPermanently:
		returner += std::to_string(
		    static_cast<int>(HTTPResponseCodes::MovedPermanently));
		returner += " ";
		returner += string_from_response_code(
				HTTPResponseCodes::MovedPermanently)
				.value();
		break;

	// For invalid  HTTP status code from a server, we return a empty string
	case HTTPResponseCodes::InvalidHttpCode:
		return "";
		break;
	}
	returner += "\r\n";
	returner += http_headers_->buildRawHeader();
	returner += std::string{raw_body_->getStartOffsetPointer(),
				raw_body_->getEndOffsetPointer()};
	return returner;
}

BLUETH_FORCE_INLINE void
HTTPResponseMessage::pushBackHeaderValue(char char_val) noexcept {
	temp_header_value_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE void
HTTPResponseMessage::pushBackHeaderName(char char_val) noexcept {
	temp_header_name_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE void
HTTPResponseMessage::addTempHeadersHolderToMessage() noexcept {
	http_headers_->addHeader({std::move(temp_header_name_holder_),
				  std::move(temp_header_value_holder_)});
	temp_header_name_holder_.clear();
	temp_header_value_holder_.clear();
}

BLUETH_FORCE_INLINE void
HTTPResponseMessage::pushBackRawBody(std::string &&raw_body) noexcept {
	raw_body_->appendRawBytes(raw_body.c_str(), raw_body.size());
}

BLUETH_FORCE_INLINE void
HTTPResponseMessage::pushBackResponseCode(char char_value) noexcept {
	if (temp_http_status_code_holder_.current_index <= 2)
		temp_http_status_code_holder_
		    .http_code_holder[temp_http_status_code_holder_
					  .current_index++] = char_value;
}

BLUETH_FORCE_INLINE const char *
HTTPResponseMessage::getTempStatusCode() const noexcept {
	return temp_http_status_code_holder_.http_code_holder;
}

} // namespace blueth::http
