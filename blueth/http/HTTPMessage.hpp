#pragma once
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
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

	// These two are only to work with the state-machine parser
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
	raw_body_->appendRawBytes(std::move(*io_buffer.get()));
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
	returner += std::string(raw_body_->begin(), raw_body_->end());
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

} // namespace blueth::http
