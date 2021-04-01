#pragma once
#include "common.hpp"
#include "http_constants.hpp"
#include "http_headers.hpp"
#include "io/IOBuffer.hpp"
#include <algorithm>
#include <memory>

namespace blueth::http {
class http_request_message {
      private:
	static constexpr std::size_t initial_capacity_ = 2048;
	std::unique_ptr<http_headers> http_headers_{nullptr};
	std::unique_ptr<io::IOBuffer<char>> raw_body_{nullptr};
	http_request_type request_type_;
	http_version http_message_version_;
	std::string target_resource_;

	// These two are only to work with the state-machine parser
	std::string temp_header_value_holder_;
	std::string temp_header_name_holder_;
	std::string temp_http_method_holder_;

      public:
	http_request_message();
	BLUETH_FORCE_INLINE static std::unique_ptr<http_request_message>
	create();
	BLUETH_FORCE_INLINE void
	add_header(std::string &&header_name,
		   std::string &&header_value) noexcept;
	BLUETH_FORCE_INLINE bool
	remove_header(std::string &&header_name) noexcept;
	BLUETH_FORCE_INLINE const std::unique_ptr<http_headers> &
	const_get_http_headers() const noexcept;
	BLUETH_NODISCARD BLUETH_FORCE_INLINE std::unique_ptr<http_headers>
	get_http_headers() noexcept;
	BLUETH_FORCE_INLINE void flush_body() noexcept;
	BLUETH_FORCE_INLINE void
	set_request_type(http_request_type type) noexcept;
	BLUETH_FORCE_INLINE http_request_type get_request_type() const noexcept;
	BLUETH_FORCE_INLINE void
	set_http_version(http_version version) noexcept;
	BLUETH_FORCE_INLINE http_version get_http_version() const noexcept;
	BLUETH_FORCE_INLINE void
	set_http_target_resource(std::string &&target_resource) noexcept;
	BLUETH_FORCE_INLINE const std::string &
	get_target_resource() const noexcept;
	void
	set_raw_body(std::unique_ptr<io::IOBuffer<char>> io_buffer) noexcept;
	BLUETH_FORCE_INLINE std::optional<std::string>
	get_header_value(std::string &&header_name) noexcept;
	std::string build_raw_message() const noexcept;

	// These methods are only implemented to work with the state-machine
	// parser
	BLUETH_FORCE_INLINE void push_back_header_value(char char_val) noexcept;
	BLUETH_FORCE_INLINE void push_back_header_name(char char_val) noexcept;
	BLUETH_FORCE_INLINE void add_temp_header_holder_to_message() noexcept;
	BLUETH_FORCE_INLINE void
	push_back_raw_body(std::string &&raw_body) noexcept;
	BLUETH_FORCE_INLINE void push_back_raw_body(char char_val) noexcept;
	BLUETH_FORCE_INLINE void
	push_back_target_resource(char char_val) noexcept;
	BLUETH_FORCE_INLINE void
	push_back_request_method(char char_val) noexcept;
	BLUETH_FORCE_INLINE std::string
	get_temp_request_method() const noexcept;
};

BLUETH_FORCE_INLINE inline std::unique_ptr<http_request_message>
http_request_message::create() {
	return std::make_unique<http_request_message>();
}

inline http_request_message::http_request_message()
    : request_type_{http_request_type::unsupported},
      http_message_version_{http_version::http_1_1},
      temp_header_value_holder_{""}, temp_header_name_holder_{""},
      target_resource_{""}, http_headers_{std::make_unique<http_headers>()},
      raw_body_{io::IOBuffer<char>::create(initial_capacity_)} {}

BLUETH_FORCE_INLINE inline void
http_request_message::add_header(std::string &&header_name,
				 std::string &&header_value) noexcept {
	http_headers_->add_header({std::forward<std::string>(header_name),
				   std::forward<std::string>(header_value)});
}

BLUETH_FORCE_INLINE inline bool
http_request_message::remove_header(std::string &&header_name) noexcept {
	return http_headers_->remove_header(
	    std::forward<std::string>(header_name));
}

BLUETH_FORCE_INLINE inline const std::unique_ptr<http_headers> &
http_request_message::const_get_http_headers() const noexcept {
	return http_headers_;
}

BLUETH_NODISCARD BLUETH_FORCE_INLINE inline std::unique_ptr<http_headers>
http_request_message::get_http_headers() noexcept {
	return std::move(http_headers_);
}

BLUETH_FORCE_INLINE inline void http_request_message::flush_body() noexcept {
	raw_body_->clear();
}

BLUETH_FORCE_INLINE inline void
http_request_message::set_request_type(http_request_type req_type) noexcept {
	request_type_ = req_type;
}

BLUETH_FORCE_INLINE inline http_request_type
http_request_message::get_request_type() const noexcept {
	return request_type_;
}

BLUETH_FORCE_INLINE inline void
http_request_message::set_http_version(http_version version) noexcept {
	http_message_version_ = version;
}

BLUETH_FORCE_INLINE inline http_version
http_request_message::get_http_version() const noexcept {
	return http_message_version_;
}

BLUETH_FORCE_INLINE inline void http_request_message::set_http_target_resource(
    std::string &&target_resource) noexcept {
	target_resource_ += std::forward<std::string>(target_resource);
}

BLUETH_FORCE_INLINE inline const std::string &
http_request_message::get_target_resource() const noexcept {
	return target_resource_;
}

inline void http_request_message::set_raw_body(
    std::unique_ptr<io::IOBuffer<char>> io_buffer) noexcept {
	raw_body_->appendRawBytes(std::move(*io_buffer.get()));
}

BLUETH_FORCE_INLINE inline std::optional<std::string>
http_request_message::get_header_value(std::string &&header_name) noexcept {
	return http_headers_->get_header_value(
	    std::forward<std::string>(header_name));
}

inline std::string http_request_message::build_raw_message() const noexcept {
	std::string returner;
	switch (request_type_) {
	case http_request_type::get:
		returner += "GET ";
		break;
	case http_request_type::post:
		returner += "POST ";
		break;
	case http_request_type::head:
		returner += "HEAD ";
		break;
	case http_request_type::put:
		returner += "PUT ";
		break;
	case http_request_type::unsupported:
		return returner;
	}
	returner += target_resource_;
	returner += " ";
	returner += "HTTP/1.1";
	returner += "\r\n";
	returner += http_headers_->build_raw_header();
	returner += std::string(raw_body_->begin(), raw_body_->end());
	return returner;
}

BLUETH_FORCE_INLINE inline void
http_request_message::push_back_header_value(char char_val) noexcept {
	temp_header_value_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline void
http_request_message::push_back_header_name(char char_val) noexcept {
	temp_header_name_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline void
http_request_message::add_temp_header_holder_to_message() noexcept {
	http_headers_->add_header({std::move(temp_header_name_holder_),
				   std::move(temp_header_value_holder_)});
}

BLUETH_FORCE_INLINE inline void
http_request_message::push_back_raw_body(std::string &&raw_body) noexcept {
	raw_body_->appendRawBytes(raw_body.c_str(), raw_body.size());
}

BLUETH_FORCE_INLINE inline void
http_request_message::push_back_raw_body(char char_val) noexcept {
	raw_body_->appendRawBytes(&char_val, 1);
}

BLUETH_FORCE_INLINE inline void
http_request_message::push_back_target_resource(char char_val) noexcept {
	target_resource_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline void
http_request_message::push_back_request_method(char char_val) noexcept {
	temp_http_method_holder_.push_back(char_val);
}

BLUETH_FORCE_INLINE inline std::string
http_request_message::get_temp_request_method() const noexcept {
	return temp_http_method_holder_;
}

} // namespace blueth::http
